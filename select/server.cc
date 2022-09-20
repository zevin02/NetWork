/*
select只负责等待，没有读取和写入的功能

以读为例
1. 用户告知内核，你要帮我关心哪些fd上的读事件就绪
2. 内核告诉用户，你所关心的哪些读事件已近就绪

比特位位置代表哪一个socket

比特位内容

输入：我们置1，就是告诉内核要关心的fd集合
输出：内核告诉用户，位图内核返回的时候，会做修改，把位图为1的就是就绪了
这是同一个位图结构

select等待策略
1. 阻塞等待，只要不就绪，就不返回
2. 非阻塞等待，只要不就绪，立马返回
3. 设置好deadline，deadline之内阻塞等待，deadline之外非阻塞等待
只要就绪就立马返回，这个叫做timeout了，超时了

timeout也是输入输出，5s之内就返回了，3s返回，它的timeout就剩下了2s，
超过5s，返回的就是0

null  阻塞
timeout=0非阻塞
timeout=n超时等待

返回值，》0有几个fd就绪了
=0超时
《0出错

因为select使用的是输入输出型参数标识不同的含义，意味着每一次，都要对fd_set进行重新设置
很麻烦
你的程序，怎么知道你都有哪些fd，要求用户把这些历史fd保存起来




*/
#include<unistd.h>
#include <sys/select.h>
#include "sock.hpp"
#define NUM (sizeof(fd_set) * 8)

int fd_array[NUM]; //这个数组的内容>=0就是合法的，-1该位置没有fd

int main(int argc, char *argv[])
{
    //最多可以存1024个fd
    if (argc != 2)
    {
        cout << "port" << endl;
        exit(1);
    }
    uint16_t port = atoi(argv[1]);
    int sockfd = Sock::Socket();
    Sock::SetSockOpt(sockfd);
    Sock::Bind(sockfd, port);
    Sock::Listen(sockfd);

    // accept是通过listensockfd来获取新连接，前提是listensock上面有新链接
    // accept不知道有新链接，所以阻塞等待
    //站在多路转接的视角，我们认为链接到来，对于listensock就是读事件
    //对于所有的服务器，最开始只有listen_sockfd

    //事件循环

    for(int i=0;i<NUM;i++)
    {
        fd_array[i]=-1;
    }
    fd_array[0] = sockfd;//把第一个位置放成监听套接字，用来保存具体的文件描述符
    fd_set rfds;
    while (1)
    {
        FD_ZERO(&rfds);                                               //每次都要重新操作
        int maxfd=fd_array[0];//最大的fd
        for(int i=0;i<NUM;i++)
        {
            if(fd_array[i]==-1)//我们规定-1就没有fd
            {
                continue;
            }
            //这里不是-1，就有套接字
            FD_SET(fd_array[i],&rfds);//设置进rfds里面
            if(fd_array[i]>maxfd)
            {
                maxfd=fd_array[i];//更新maxfd
            }
        }
        struct timeval timeout = {5, 0};                              //超时等待5s
        int n = select(maxfd+1, &rfds, nullptr, nullptr, nullptr); //暂时阻塞,select的等，一次可以等多个fd
        // accept一次只能等待一个fd，
        // recv和send accept只负责自己最核心的工作，真正的读写
        switch (n)
        {
        case -1:
            cout << "select error" << endl;
            break;
        case 0://超时
            cout << "timeout" << endl;
            break;
        default:
            cout << "有对应的fd" << endl;//不读就会一直通知
            for(int i=0;i<NUM;i++)
            {
                //再去检测一下
                if(fd_array[i]==-1)continue;//不合法就结束
                
                //这边的fd都是合法的fd,合法的fd不一定是就绪的fd
                if(FD_ISSET(fd_array[i],&rfds))
                {
                    //检测一个fd是否在集合里面被设置
                    //因为它是输出型参数
                    //一定是读事件就绪了

                    cout<<"sock"<<fd_array[i]<<"上面有读事件可以读取了"<<endl;
                    if(fd_array[i]==sockfd)
                    {
                        cout<<"有新连接到来"<<endl;
                        int newsock=Sock::Accept(fd_array[i]);
                        //这里已近成功获取了一个新链接，但是不能直接读取
                        //因为这个链接的读写事件并没有就绪
                        //把这个放到数组里面
                        int pos=1;
                        for(pos=1;pos<NUM;pos++)//从1开始，因为0是listen_fd
                        {
                            if(fd_array[pos]==-1)
                            {
                                //这个位置没有被设置
                                break;
                            }
                        }
                        if(pos<NUM)
                        {
                            cout<<"已近被添加到数组里面了"<<endl;
                            fd_array[pos]=newsock;
                        //1.找到一个位置没有被使用
                        }
                        else
                        {
                        //2.找到了所有fd_array[],都没有找到没有被使用的位置
                            //服务器已近满载了
                            cout<<"服务器已经满载了"<<endl;
                            close(newsock);//关闭连接，让它重新链接
                        }

                    }
                    else
                    {
                        //read,write
                        //一个连接退出的时候也会被当作读事件
                        //可是，本次读取一定会读完吗，读完一定不会粘包吗
                        //但是，我们今天无法解决，没有场景
                        cout<<"sock:"<<fd_array[i]<<"上面有普通读取"<<endl;

                        char buf[1024]={0};
                        ssize_t s=recv(fd_array[i],buf,sizeof(buf)-1,0);//这样会出现粘包
                        if(s>0)
                        {
                            buf[s]=0;
                            cout<<"client:"<<buf<<endl;
                        }
                        else if(s==0)
                        {
                            //读到0，对端连接关闭
                            //我们就要关闭
                            cout<<"sock:"<<fd_array[i]<<"关闭链接"<<endl;
                            close(fd_array[i]);
                            //把它从数组去掉
                            fd_array[i]=-1;
                            cout<<"已近在数组中去掉了下标"<<endl;
                            
                        }
                        else
                        {
                            //读取失败
                            cout<<"断开连接"<<endl;
                            close(fd_array[i]);
                            //把它从数组去掉
                            fd_array[i]=-1;
                            cout<<"已近在数组中去掉了下标"<<endl;
                        }


                    }
                }

            }
            break;
        }
    }

    return 0;
}

//优缺点
/*
    优点：可以一次等待多个fd，可以让我们等待的事件重叠，提高IO效率，多线程是被调度使用的，排队运行，select排队成本低


    缺点：
    1. 每次都要重新设置要关心的fd，一个就绪了都要遍历，类似每次完成之后都要遍历检测，哪个好了

    2. fd_set，它能够让select检测的fd是有上限的
    3. select底层需要轮询式的检测哪些fd就绪了，所以maxfd+1就是最大的上限
    4. select要较为搞频率的内核到用户，用户到内核的频繁拷贝


*/