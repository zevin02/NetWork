#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>

/*
    这里我们实现epoll的et模式
    1.缓冲区剩余的数据没有读全，不会触发wait，等到有新的事件来的时候，才会触发
    struct epoll_event event;
    event.events=EPOLLIN|EPOLLET;//这样设置就是ET模式

    2.epoll的ET模式只支持非阻塞模式（文件）
    所以我们连接上来的文件描述符都要把它设置成非阻塞状态
    int flag=fcntl(cfd,F_GETFD);
    fcntl(cfd,F_SETFL,flag|O_NONBLOCK)；
*/
#if 0
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
    ev.events = EPOLLIN|EPOLLET;                           //事件,读入
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
                //这里我们把新获得的文件描述符设置成非阻塞状态
                int flag=fcntl(clientfd,F_GETFL);
                fcntl(clientfd,F_SETFL,flag|O_NONBLOCK);
                //这样就是变成了ET模式
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
#endif

//我们这之后只要把监听套接字设置成et，把接收上来的文件描述符设置成非阻塞

#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <iostream>

using namespace std;

int main()
{

    //设置文件描述符的读写属性为非阻塞属性
    int flag = fcntl(0,F_GETFL);
    if(flag < 0)
    {
        cout << "fcntl failed" << endl;
        return -1;
    }
    
    //设置非阻塞属性
    fcntl(0,F_SETFL,flag | O_NONBLOCK);

    int epoll_handle = epoll_create(2);
    if(epoll_handle < 0)
    {
        cout << "epoll_create failed" << endl;
        return 0;
    }
    
    struct epoll_event ee;
    //设置对当前文件描述符的监控为可读事件和ET模式
    ee.events = EPOLLIN | EPOLLET;
    ee.data.fd = 0;
    int ret = epoll_ctl(epoll_handle,EPOLL_CTL_ADD,0,&ee);
    if(ret < 0)
    {
        cout << "epoll_ctl failed" << endl;
        return 0;
    }

    while(1)
    {
        struct epoll_event arr[2];
        ret = epoll_wait(epoll_handle,arr,2,-1);
        if(ret < 0)
        {
            cout << "epoll_wait failed" << endl;
            return 0;
        }

        for(int i = 0; i < 2; ++i)
        {
            if(arr[i].data.fd == 0)
            {
                string res = "";
                while(1)
                {
                    char buf[3] = {0};
                    int t = read(arr[i].data.fd,buf,sizeof(buf)-1);
                    if(t < 0)
                    {   //注意：这里当返回值小于0的时候，不一定是发生了错误
                        //由于文件描述符是非阻塞属性，因此，每次读的时候都会返回一个值
                        //若返回的错误码是EAGAIN或者EWOULDBLOCK的时候，说明此时已经将缓冲区中的内容读完了
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                    }
                    res += buf;
                }
                cout << res << endl; 
            }
        }
    }

    return 0;
}
