#pragma once
#include "reactor.hpp"
#include <vector>
#include <cerrno>
#include "Util.hpp"
//这里就是业务细节

#define ONCE_SIZE 128

// 1.本轮读取完成
//-1.读取出错
// 0.对端关闭链接

static int RecverCore(int sock, string &inbuff)
{
    //这个地方就是主要的读取数据
    while (true)
    {
        //必须把这一轮的数据都读取上来
        char buffer[ONCE_SIZE];                           //每次IO 的基本大小都是128
        ssize_t s = recv(sock, buffer, ONCE_SIZE - 1, 0); //这里我们把它当作字符串看待
        if (s > 0)
        {
            //读取成功，还要继续一直读
            buffer[s] = 0;
            inbuff += buffer;
        }
        else if (s < 0)
        {
            //阻塞状态，S状态可中断，
            // 1.读完底层没数据,就结束循环了
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //这次的读取，已经读完，底层没数据
                return 1; //这里就是成功的了 success
            }
            // 2.真正出错了
            else if (errno == EINTR)
            {
                //在IO 的时候被信号打断，但是概率特别低
                continue; //被信号打断了，就继续回来，再读一次，从数据里面读取
            }
            else
            {
                //这里就真的出错了
                return -1;
            }
        }
        else
        {
            // s=0
            //对方退出链接了
            return 0;
        }
    }
}

int Recver(Event *evp)
{
    cout << "Recv Callback has been called" << endl;
    //我们要循环一直读，一直读到recv出错的时候，才会结束读取
    // 1. 读取成功就一直读
    // 2.读取出错
    // 2.a非阻塞的,一定回出错，触发那个标志，就是结束了
    // 2.b真正的读取出错

    // 1.真正的读
    int result = RecverCore(evp->sockfd, evp->inbuff); //我们从evp->fd里面读取数据都读取到它对应的buff里面

    if (result <= 0)
    {
        //读取出错或者关闭连接
        //进行差错处理
        if (evp->errer)
        {
            //调用差错处理方法
            evp->errer(evp); //所有的差错处理，就这一个方法就可以了,所有的错误元都统一到这个函数里面
        }
        return -1;
    }

    //假设我们一个一个请求是这样的
    // 1+2X3*1X5-0X

    //这下面的代码就是根据我们世纪的业务逻辑，可以进行替换的
    //在这个里面可以引入线程池,专门可以进行业务处理





    // 2.我们要先分包，因为我们不知道读的数据是不是完整的报文，往下交付的都是一个一个完整的报文
    vector<string> tokens; //这里面存储的就是一个一个有效的报文,如果inbuff里面有不完整的报文，那个不完整的数据就留在inbuff里面

    SplitSegment(evp->inbuff, &tokens, "X"); //将数据进行拆分,这里我们认为分隔符是'X'

    //这里后面也可以再建立一层

    // 3.我们要针对一个报文进行反序列化，提取有效报文参与存储的信息
    for (auto &seg : tokens)
    {
        string data1, data2;
        // 4.我们要进行业务逻辑处理
        if (DeSerialize(seg, &data1, &data2))//这个地方进行了反序列化

        {
            //反序列化,这个就是和业务强相关了
            int x = stoi(data1);
            int y = stoi(data2);
            //这里把任务push到线程池里面，处理完之后，设计一个处理完拿回任务结果的接口

            int r = x + y;


            // 5.构建响应----把所有的数据添加到evp->outbuff里面
            string res = data1 + "+" + data2+"="+to_string(r) + "X"; //构建响应报文也要加上分隔符



            //把所有从线程池里面拿回来的结果拼到outbuff里面，然后就不用管了
            evp->outbuff += res;                    //把数据弄到，我们的发送缓冲区里面,用户层缓冲区我们自己定义的
        }
    }

    // 6.尝试直接间接进行发送---后续说明

    // TODO,真正触发Reactor让它给我们去发送数据
    //必须条件成熟了（写事件就绪），你才能发送，
    //一般只要将报文处理完了，才需要发送
    //写事件一般基本都是就绪的，但是用户不一定是就绪的！
    //对于写事件，我们通常是安需设置的，按用户的需要来设置
    if (!(evp->outbuff).empty())
    {
        //发送缓冲区里面有数据
        evp->R->EnableRW(evp->sockfd, true, true); //缓冲区有数据，我们就设置成能够写就绪
        //写打开的时候，默认就是就绪的
        //即便是缓冲区已经满了，epoll规定，只要用户重新设置了OUT事件，EPOLLOUT至少会再触发一次
        // epoll当前就会发送了

        //一旦我们打开了，它对应的outbuf里面有数据，事件派发起，就关心对应的写事件了，会自动调用发送函数
        //会自动调用回调方法，把数据发送出去
    }

    //这里还可以再写一个软件层就行了
}

static int SendCore(int sock, string &outbuff)
{
    while (true)
    {
        int total = 0; //本轮累计发送的数据量
        const char *start = outbuff.c_str();
        int size = outbuff.size();                                 //需要发送的总大小，这里我们直接发送即可，因为已经进行了解决粘包的处理
        ssize_t curr = send(sock, start + total, size - total, 0); // start+total从total后的位置开始发送,剩余需要发送的数据就是size-total
        // curr就是本次真正发送的字节数
        if (curr > 0)
        {
            total += curr; // total每次都要把一次读取的字节大小记录起来
            if (total == size)
            {
                //把所有数据都读完了
                outbuff.clear(); //发送完之后，把这个outbuff里面的数据都清理掉
                return 1;        //发送成功
            }
        }
        else
        {
            if (errno == EINTR)
            {
                //再发送的过程中，被信号给中断了
                continue; //直接就继续下面的发送
            }
            //数据没有发完，但是不能再发了，非阻塞
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //把已经发送的数据先删除掉
                outbuff.erase(0, total); //
                return 0;
            }
            return -1; //发送失败
        }
    }
}

// 1.全部发送的数据发送成功
// 0.数据没有发送完，但是不能再发送了
//-1.发送失败
int Sender(Event *evp)
{
    cout << "Sender been called" << endl;

    //无脑向缓冲区 里面写数据
    //如果写不成功，那就继续写
    //如果写成功，写完了，把写就绪关掉
    int result = SendCore(evp->sockfd, evp->outbuff); //发送数据
    if (result == 1)
    {
        evp->R->EnableRW(evp->sockfd, true, false); //发送完之后，就把写事件给关掉,按需设置
    }
    else if (result == 0)
    {
        //你可以什么都不做
        evp->R->EnableRW(evp->sockfd, true, true);
    }
    else
    {
        //发送失败
        if (evp->errer)
        {
            evp->errer(evp); //发送失败，就调用自己注册的异常事件,让它处理自己的
        }
    }
}

//所有的差错处理都再这里
int Errer(Event *evp)
{
    cout << "Errer been called" << endl;
    evp->R->DeleteEvent(evp); //出现异常了，就该把对应的事件给取消掉
}