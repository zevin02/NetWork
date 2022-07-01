#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/epoll.h>

/*
    在select里面我们对于那些事件要不断的拷贝，在epoll里面就不需要了
*/


/*

    1.创建一个epoll的句柄，它本身就是一个fd
    int epoll_creat(int size)
    这里的size只要是一个>0的值就可以了，多少不用我们来关心


    2.注册需要监视fd和事件
    int epoll_ctl(int epfd,int op,int fd,struct epoll_event* event)

    * op就是操作的类型 ADD，：需要监听什么就把它加进去
                      DEL：不需要这个fd就把体删除
                      MOD：把它这个fd给修改掉


    * struct epoll_event
    {
        uint32_t events;//监听的事件
        //这也是EPOLLIN,EPOLLOUT,EPOLLERR
        epoll_data_t data;//用户可用的文件描述符
    }
    struct epoll_data
    {
        int fd;
    }


    3.等待事件的发生
    int epoll_wait(int epfd,struct epoll_event* events,int maxevents,int timeout)

    我们去等待，如果有事件发生，我们就把事件放到events数组里面
    maxevent 必须要大于0，要等待的最大个数
    timeout是阻塞的最小时间，-1就是阻塞等待，0就是非阻塞等待


    返回值：
    成功的话，就返回有多少个文件描述符已近准备好了，可以等待我们进行操作
    失败返回-1，错误码会被设置成合适的值

*/
int initserver(int port)
{
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        exit(1);
    }
    int opt=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(opt));
    struct sockaddr_in peer;
    memset(&peer,0,sizeof(peer));
    peer.sin_family=AF_INET;
    peer.sin_port=port;
    peer.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(bind(sockfd,(struct sockaddr*)&peer,sizeof(peer))<0)
    {
        exit(1);
    }
    if(listen(sockfd,5)<0)
    {
        exit(1);
    }
    return sockfd;
}


int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        printf("the parmeter lost \n");
        return 1;
    }
    int sockfd=initserver(atoi(argv[1]));
    //创建一个文件描述符
    int epolfd=epoll_create(1);
    //添加一个监听的文件描述符
    struct epoll_event ev;
    ev.data.fd=sockfd;//监听的文件描述符
    ev.events=EPOLLIN;//事件,读入
    epoll_ctl(epolfd,EPOLL_CTL_ADD,sockfd,&ev);//这里先把监听的文件描述符先添加进去
    while(1)
    {
        struct epoll_event events[10];//这个结构体里面存放着有事件发生的结构体数组
        //等待监听的sockfd有事件发生
        int infds=epoll_wait(epolfd,events,10,-1);//-1代表就要去阻塞等待
        if(infds<0)
        {
            perror("infds");
            exit(1);
        }
        else if(infds==0)
        {
            printf("超时");
            continue;
        }
        //调用完wait之后，它就会把event结构体里面的所有东西都安排好，所以它返回的个数，都是发生事件了的
        //遍历有事件发生的结构体数组,
        for(int i=0;i<infds;i++)
        {
            //如果是一个监听的sockfd,发生的读事件，就说明有客户端连接上来
            if((events[i].data.fd==sockfd)&&(events[i].events&EPOLLIN))
            {
                //表示客户端有连接请求发过来
                struct sockaddr_in client;
                socklen_t len=sizeof(client);
                int clientfd=accept(sockfd,(struct sockaddr*)&client,&len);
                if(clientfd<0)
                {
                    exit(1);
                }
                //把新的客户端添加到epoll里面去
                memset(&ev,0,sizeof(ev));
                ev.data.fd=clientfd;
                ev.events=EPOLLIN;
                epoll_ctl(epolfd,EPOLL_CTL_ADD,clientfd,&ev);//添加一个文件描述符
                continue;
            }
            else if(events[i].events&EPOLLIN)
            {
                //其他文件描述符发送数据过来
                char buf[1024];
                ssize_t size=read(events[i].data.fd,buf,sizeof(buf));
                if(size<=0)
                {
                    //发送错误，连接断开
                    //现在我们就只需要把断开的客户端从epoll里面删除掉
                    memset(&ev,0,sizeof(ev));
                    ev.data.fd=events[i].data.fd;//把这个文件描述符添加进去
                    ev.events=EPOLLIN;
                    epoll_ctl(epolfd,EPOLL_CTL_DEL,sockfd,&ev);//删除这个文件描述符
                    close(events[i].data.fd);//关闭
                    
                }
            }
        }
    }

    return 0;
}