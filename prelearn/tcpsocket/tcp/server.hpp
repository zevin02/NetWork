#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <cstring>
#include <arpa/inet.h>
#include <string>
#include <pthread.h>
#include <cstdlib>
#include "threadpool.hpp"
using namespace ns_threadpool;
using namespace std;
#define MAX 1024
#define DEFUALTPORT 8081
#define BACKLOG 20

//因为我们要获取里面的东西，所以定义称struct类型的就可以了

//设计一个handler类，在handler类里面对（）操作符进行重载，将（）操作符的执行动作重载为执行server函数的代码
// class Handler
// {
// public:
//     Handler()
//     {
//     }
//     ~Handler()
//     {
//     }
//     void operator()(int sock, string cliip, int cliport)
//     {
//         //执行server函数的代码
//         char buff[1024];
//         while(true)
//         {
//             ssize_t size=read(sock,buff,sizeof(buff)-1);
//             if(size>0)//读取成功
//             {
//                 buff[size]='\0';
//                 cout<<cliip<<":"<<cliport<<"#"<<buff<<endl;
//             }
//             else if(size==0)//对端关闭了
//             {
//                 cout<<cliip<<":"<<cliip<<" close!"<<endl;
//             }
//             else
//             {
//                 //读取失败
//                 cerr<<sock<<" read error!"<<endl;
//                 break;//读取失败的化就关闭
//             }
            
//         }
//             close(sock);//这个线程用完了就要把这个文件描述符关掉
//             cout<<cliip<<":"<<cliport<<" service done!"<<endl;
//     }
// };

// class Task //任务
// {
// private:
//     int _sockfd;
//     string _cliip;
//     int _cliport;
//     Handler _handler; //处理方法
// public:
//     Task() //无参数，就是为了线程池取任务出来执行
//     {
//     }
//     Task(int sock, string ip, int port) //构造函数里面放任务
//         : _sockfd(sock), _cliip(ip), _cliport(port)
//     {
//     }
//     ~Task()
//     {
//     }

//     //处理任务的函数
//     void Run()
//     {
//         _handler(_sockfd, _cliip, _cliport); //调用仿函数
//     }
// };

struct Param
{
    int _sockfd;
    string _ip;
    int _port;
    Param(int sockfd, string ip, int port)
        : _sockfd(sockfd), _ip(ip), _port(port)
    {
    }
    ~Param()
    {
    }
};

//我们在服务端引入了线程池，在服务类里面增加一个线程池的指针成员
class TCPSERVER
{
private:
    int _sockfd; //监听套接字
    int _port;   //服务端的端口号
    ThreadPool<Task> *tp;

public:
    void err(const char *error)
    {
        perror(error);
        exit(1);
    }
    TCPSERVER(int port = DEFUALTPORT)
        : _port(port), _sockfd(-1)
    {
    }
    ~TCPSERVER()
    {
        if (_sockfd > 0)
            close(_sockfd);
    }
    void InitServer()
    {
        signal(SIGCHLD, SIG_IGN); //忽略子进程退出后，给父进程发送的信号
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            err("socket");
        }
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_port = htons(_port);
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = INADDR_ANY;
        if (bind(_sockfd, (struct sockaddr *)&local, sizeof(local)) < 0)
        {
            err("bind");
        }
        if (listen(_sockfd, BACKLOG) < 0)
        {
            err("listen");
        }
        tp = new ThreadPool<Task>(); //先动态开辟一个线程池对象
    }
    static void *Routine(void *args) //因为在类里面，所以不要this指针
    {
        Param sock = *(Param *)args;
        delete args;
        pthread_detach(pthread_self()); //线程分离就不需要再去join了
        cout << "create a new pthread for IO" << endl;
        char buff[MAX];
        while (true)
        {
            //读取数据
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            ssize_t s = recvfrom(sock._sockfd, buff, sizeof(buff) - 1, 0, (struct sockaddr *)&peer, &len); // peer里面就是远程的数据了
            if (s > 0)
            {
                buff[s] = 0;
                fflush(stdout);
                cout << "#client:" << buff << endl;
                string msg = "#client";
                msg += buff;
                sendto(sock._sockfd, msg.c_str(), msg.size(), 0, (struct sockaddr *)&peer, len);
            }
            else
            {
                cout << "error data" << endl;
                break;
            }
        }
    }
    void StartTcp()
    {
        tp->InitThreadPool(); //初始化线程池

        while (true)
        {
            sockaddr_in endpoint;
            memset(&endpoint, '\0', sizeof(endpoint));
            //处理任务
            socklen_t len = sizeof(endpoint);
            int sock = accept(_sockfd, (struct sockaddr *)&endpoint, &len); //这个sock就是用来和客户端进行通信的，
            if (sock < 0)
            {
                perror("accept");
                continue;
            }
            string cli_ip = inet_ntoa(endpoint.sin_addr);
            string ip(cli_ip);
            int port = ntohs(endpoint.sin_port);
            cli_ip += ":";
            cli_ip += to_string(ntohs(endpoint.sin_port));
            cout << "get a link->" <<sock<< cli_ip <<"->"<<port<< endl;
            // pthread_t tid;
            //但是调用server函数的时候我们需要的是socket ip port，创建线程的时候只能传一个参数，所以我们需要定义一个结构体存储这些信息
            // Param *p = new Param(sock, ip, port); //这样就把数据都初始化传进去了，三个数据都有了

            //对于主线程accept上来的文件描述符，不能由主线程来close，应该让操作这个sockfd的线程进行关闭
            //同样，新线程也不能对监听sockfd进行关闭
            // pthread_create(&tid, nullptr, Routine, (void *)p); //在这里执行操作，所以这个地方就要传入套接字

            //构造一个任务
            Task task(sock,ip,port);
            //把这个任务放到线程池的任务队列里面取
            tp->PushTask(task);


            /*
                各个线程共享一个文件描述符表，因此当主线程调用accept的时候，其他创建的新线程是可以直接访问到这个文件描述符的
                由于此时新线程并不知道它要服务的客户端所要对应的是哪一个文件描述符，所以此时主线程要告诉新线程应该要去访问的文件描述符的值
                就是要操作哪一个套接字

            */
            /*
            当前多线程版本的服务器存在的问题是，每次有新的链接来的话，就会为客户端创建为它服务的新线程，在服务结束之后就会把他进行销毁，
            这样不仅有麻烦，效率还很低下

            如果有大量的客户端进行连接请求，此时服务端要为每一个客户端创建对应的服务线程，计算机中的线程越多，CPU 的压力也就越大，CPU要在
            这些线程里面不断的来回切换，此时CPU 在调度线程的时候，线程和线程之间的切换成本就很高，线程如果很多的话，被调度的周期也就很长，
            这样客户端也会迟迟得不到应答
            */

            /*
            解决方案
            * 可以先在服务端创建一些线程，当有客户端请求连接的时候 就让这些线程提供服务，此时客户端一来就有线程来替他们服务，而不是客户端请求了才创建线程

            * 当某个线程为客户端提供完服务之后，不要让该线程退出，而是让该线程继续为下一个客户端提供服务，如果当前没有客户端连接请求，则可以让线程先休眠
            当有客户端到来的时候再把该线程唤醒

            * 服务端创建的这一批线程不能太多，此时CPU 的压力也就不会太大，如果此时客户端连接到来，此时一批线程都在进行工作，那么此时服务端不应该再创建线程
            而是让这个新来的线程再连接队列里面排队，等服务端这些线程有空闲的就替它提供服务



            此时就引入了线程池再服务端里面，因为线程池的存在就是为了避免处理短时间任务创建和销毁线程的代价，此时线程池还能够保证内核充分利用，避免过分调度

            其中线程池里面有一个任务队列，当有新的任务来的时候，就把任务push到线程池里面，再线程池里面我们默认创建了5个线程，如果线程池里面没有那么多任务，就休眠



            */
        }
    }
};