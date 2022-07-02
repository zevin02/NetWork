#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include<unistd.h>

/*
    在select里面我们对于那些事件要不断的拷贝，在epoll里面就不需要了
*/

/*

    epoll就是就绪事件通知方案（句柄：唯一标识某种系统资源的标识符）

    1.创建一个epoll的操作句柄，它本身就是一个fd
    int epoll_creat(int size)
    这里的size只要是一个>0的值就可以了，多少不用我们来关心


    2.注册需要监视fd和事件
    int epoll_ctl(int epfd,int op,int fd,struct epoll_event* event)

    * epfd就是epoll操作句柄

    * op就是操作的类型 ADD，：需要监听什么就把它加进去
                      DEL：不需要这个fd就把体删除
                      MOD：把它这个fd给修改掉

    * fd就是要处理的文件描述符（增加，删除，修改）
    * struct epoll_event
    {
        uint32_t events;//监听的事件
        //这也是EPOLLIN,EPOLLOUT,EPOLLERR
        epoll_data_t data;//用户可用的文件描述符
    }

     typedef union epoll_data
     {
               void        *ptr;
               int          fd;
               uint32_t     u32;
               uint64_t     u64;
           } epoll_data_t;
    
    这是一个联合体，主要关注ptr和fd两个，也是让程序员使用的，在使用的时候必须要保护fd，
    我们这里传入的文件描述符和epoll_ctl参数中传入的fd应该保持一致，
    struct epoll_data
    {
        int fd;
    }

    成功返回0，失败返回-1


    3.等待事件的发生
    int epoll_wait(int epfd,struct epoll_event* events,int maxevents,int timeout)

    从底层就绪队列中获取事件

    我们去等待，如果有事件发生，我们就把事件放到events数组里面
    
    * epfd操作句柄
    * event和ctl一样，不过这里是事件的集合数组，是从epoll当中获取就绪的事件结构
    
    maxevent 必须要大于0，要等待的最大个数
    timeout是阻塞的最小时间，-1就是阻塞等待，0就是非阻塞等待


    返回值：
    成功的话，就返回有多少个文件描述符已近准备好了，可以等待我们进行操作
    失败返回-1，错误码会被设置成合适的值

*/




/*

    注意：
    * epoll_create创建epoll模型，用完返回一个文件描述符，使用完要close关闭掉
    * 定义的数组的意义不同了，用户要定义一个数组，但是这个数组对比poll，它不作为一个管理就绪文件描述符的工作，而是对存放每次的事件就绪的文件描述符，并且返回值就是就续保的个数
    所以当我们对这个数组使用，每次都是有效的操作
    * 要关心的文件描述符，就绪的文件描述符都有内核维护，底层通知上层的时候，只需要提供缓冲区供给OS自动会自动拷贝进来，保证了每次O（1）的检测事件是否就绪，
    O（N）获取全部就绪的事件，


    epoll模型

    * 一颗红黑树，节点上主要关心文件描述符fd和文件描述对应的事件
    * 注册网卡驱动的回调函数，当事件就绪的时候，就会调用网卡驱动方法，主要工作就是创建就绪队列的节点，告诉就绪队列节点新增加一个
    * 就绪队列，队列里面存放的用户需要关心的事件
    * 其中红黑树，就绪队列，回调机制构成了epoll模型
     
    我们创建一个epoll句柄，把我们想要的文件描述符添加进去，放到红黑树里面，当在这颗红黑树里面有事件就绪了，就会通过网卡调用回调方法，高数就绪队列，增加一个节点

    红黑树的键值就是文件描述符，天然不会重复


*/
int initserver(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        exit(1);
    }
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
    struct sockaddr_in peer;
    memset(&peer, 0, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_port = port;
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(sockfd, (struct sockaddr *)&peer, sizeof(peer)) < 0)
    {
        exit(1);
    }
    if (listen(sockfd, 5) < 0)
    {
        exit(1);
    }
    return sockfd;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("the parmeter lost \n");
        return 1;
    }
    int sockfd = initserver(atoi(argv[1]));
    //创建一个文件描述符
    int epolfd = epoll_create(1);
    //添加一个监听的文件描述符
    struct epoll_event ev;
    ev.data.fd = sockfd;                           //监听的文件描述符
    ev.events = EPOLLIN;                           //事件,读入
    epoll_ctl(epolfd, EPOLL_CTL_ADD, sockfd, &ev); //这里先把监听的文件描述符先添加进去
    while (1)
    {
        struct epoll_event events[10]; //这个结构体里面存放着有事件发生的结构体数组
        //等待监听的sockfd有事件发生
        int infds = epoll_wait(epolfd, events, 10, -1); //-1代表就要去阻塞等待,infds就是有多少个文件描述符有读写操作
        if (infds < 0)
        {
            perror("infds");
            exit(1);
        }
        else if (infds == 0)
        {
            printf("超时");
            continue;
        }
        //调用完wait之后，它就会把event结构体里面的所有东西都安排好，所以它返回的个数，都是发生事件了的
        //遍历有事件发生的结构体数组,
        for (int i = 0; i < infds; i++)
        {
            //如果是一个监听的sockfd,发生的读事件，就说明有客户端连接上来
            if ((events[i].data.fd == sockfd) && (events[i].events & EPOLLIN))
            {
                //表示客户端有连接请求发过来
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(sockfd, (struct sockaddr *)&client, &len); //建立连接
                if (clientfd < 0)
                {
                    exit(1);
                }
                //把新的客户端添加到epoll里面去
                memset(&ev, 0, sizeof(ev));
                ev.data.fd = clientfd;
                ev.events = EPOLLIN;                             //读事件
                epoll_ctl(epolfd, EPOLL_CTL_ADD, clientfd, &ev); //添加一个文件描述符
                continue;
            }
            else if (events[i].events & EPOLLIN) //不是监听文件描述符，普通文件描述符就有IO操作
            {
                //其他文件描述符发送数据过来
                char buf[1024];
                ssize_t size = read(events[i].data.fd, buf, sizeof(buf));
                if (size <= 0)
                {
                    //发送错误，连接断开
                    //现在我们就只需要把断开的客户端从epoll里面删除掉
                    memset(&ev, 0, sizeof(ev));
                    ev.data.fd = events[i].data.fd; //把这个文件描述符添加进去
                    ev.events = EPOLLIN;
                    epoll_ctl(epolfd, EPOLL_CTL_DEL, ev.data.fd, &ev); //删除这个文件描述符
                    close(events[i].data.fd);                          //关闭
                    // close后会自动调用关闭文件描述符的，我们不写上述操作也是可以的
                    continue;
                }
                buf[size] = 0;
                printf("hello %s\n", buf);
                write(events[i].data.fd, buf, sizeof(buf));
            }
        }
    }
    close(sockfd);

    return 0;
}