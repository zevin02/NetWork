#include "sock.hpp"
#include <unistd.h>
#include <sys/epoll.h>
/*

    这里我们会使用epoll_data_t 中的void*ptr
    这里的data实际上它是一个联合体，我们只需要第一个参数就可以了，有了这个void* 指针之后，我们就可以在注册sockfd的时候
    传进去我们想要的参数，在wait出来的时候，我们可以用我们自己的函数来进行处理，
    这个ptr是给用户自用使用的，epoll不关心里面的东西，用户可以用epoll_data这个union在epoll_event里面附带一些自定义的消息
    这个epoll_data会随着epoll_wait返回的epoll_event一起返回，那附带的消息，就是ptr指向的自定义消息


    编码注意：
    * 一个文件描述符，需要有读和写的缓冲区
    * 我们自己定义的协议，当检测到data里面ptr为空的时候，就是监听套接字


*/

struct Bucket
{
    int fd;
    char buf[200]; //协议,里面存放的就是对端发送过来的数据
    int pos;
    Bucket(int _fd)
        : fd(_fd), pos(0)
    {
    }
};

class Server
{
private:
    int _sockfd;
    int _epollfd; // epoll模型
    int _port;

public:
    Server()=default;
    Server(int port)
        : _port(port), _epollfd(-1), _sockfd(-1)
    {
    }
    ~Server()
    {
        close(_sockfd);
    }
    void InitServer()
    {
        _sockfd = Sock::Socket();
        Sock::SetSockOpt(_sockfd);
        Sock::Bind(_sockfd, _port, "127.0.0.1");
        Sock::Listen(_sockfd);
        //创建epoll模型
        _epollfd = epoll_create(1);
    }
    void Addfd2EpollFd(int fd, uint32_t event) //使用fd的话就可以多次利用
    {
        //设置事件
        struct epoll_event ev;
        ev.data.fd = _sockfd; //将监听描述符添加到红黑树里面去，监听
        ev.events = event;    //我们想要的是读事件
        if (fd == _sockfd)
        {
            //这里我们规定ptr为nullptr的是_sockfd
            //因为data里面是有两个数据的
            ev.data.ptr = nullptr;
        }
        else if (event & EPOLLIN) //如果是读事件的话
        {
            //其他事件获取到缓冲区和fd存放
            ev.data.ptr = (void *)new Bucket(fd);
        }
        //放入一个节点到红黑树
        //内部应该是会拷贝该节点--bug
        if (epoll_ctl(_epollfd, EPOLLIN, fd, &ev) < 0) //把他添加进去
        {
            perror("epoll_ctl");
        }
    }
    void Delfd(int fd)
    {
     
        epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, nullptr);//因为这里我们要多次使用，输入输出，所以这里才设置成为空指针
        close(fd);
    }
    void HandEvent(struct epoll_event *event, int num)
    {
        //就绪队列中的event是从0开始放的
        for (int i = 0; i < num; i++)
        {
            uint32_t status = event[i].events;          // status 里面存放的是这个文件描述需要操作的状态
            Bucket *data = (Bucket *)event[i].data.ptr; //用ptr初始化
            //这里打data里面就存放了文件描述符和一些信息
            if (status & EPOLLIN)//读入
            {
                //监听和接收数据
                if (data == nullptr)
                {
                    //因为我们前面规定了如果ptr为空的话就代表了他是一个监听文件描述符
                    int fd = Sock::Accept(_sockfd); //这里我们就要建立连接，并且把这个文件描述符添加到epoll里面
                    Addfd2EpollFd(fd, EPOLLIN);
                }
                else
                {
                    //发送端向服务端发送数据过来
                    ssize_t s = recv(data->fd, data->buf + data->pos, sizeof(data->buf) - data->pos, 0); //阻塞等待,同时解决了粘包问题
                    if (s == 0)
                    {
                        //对端关闭
                        Delfd(data->fd);
                        delete data;
                    }
                    else if (s > 0)
                    {
                        //本次读取成功
                        data->pos += s; // pos往后走
                        cout << "本次读取数据 " << data->buf << endl;
                        if (data->pos >= (int)sizeof(data->buf))
                        {
                            //pos数目超过了buf的值
                            //说明此时的数据已经都放在红黑树上面了，可以拿去了,因为可能会有粘包问题导致数据拿去不了
                            //直接改变属性发送即可
                            struct epoll_event newev;
                            newev.events=EPOLLOUT;
                            newev.data.ptr=data;
                            //让发送的时候也能够使用这个控制是否读完
                            data->pos=0;
                            epoll_ctl(_epollfd,EPOLL_CTL_MOD,data->fd,&newev);
                            


                        }
                    }
                }
            }
            else if(status&EPOLLOUT)
            {
                //这里我们也要考虑粘包问题
                ssize_t s=send(data->fd,data->buf+data->pos,sizeof(data->buf)-data->pos,0);
                if(s>0)
                {
                    data->pos+=s;
                }
                else if(s==0)
                {
                    Delfd(data->fd);
                    delete data;
                }
                else
                {
                    perror("recv");
                    Delfd(data->fd);
                    delete data;
                }
            }

        }
    }
    void StartServer()
    {
        //将_sockfd放到红黑树里面去监听
        Addfd2EpollFd(_sockfd, EPOLLIN);
        //循环等待红黑树里面是否有事件发生
        while (true)
        {
            // OS-》user

            struct epoll_event eventss[20];                    //出来的数组
            int ret = epoll_wait(_epollfd, eventss, 20, 1000); //我们想要一次处理20个请求,超时时间1000s
            if (ret < 0)
            {
                perror("epoll_wait");
                exit(1);
            }
            else if (ret == 0)
            {
                cout << "time out...." << endl;
                continue;
            }
            else
            {
                //有事情要我们去做，所以处理事情
                HandEvent(eventss, ret);
            }
        }
    }
};