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
using namespace std;
#define MAX 1024
#define DEFUALTPORT 8081
#define BACKLOG 20

//因为我们要获取里面的东西，所以定义称struct类型的就可以了
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

class TCPSERVER
{
private:
    int _sockfd;
    int _port;

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
        signal(SIGINT,SIG_IGN);
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
    }
    static void *Routine(void *args) //因为在类里面，所以不要this指针
    {
        Param sock = *(Param*)args;
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
        sockaddr_in endpoint;
        memset(&endpoint,'\0',sizeof(endpoint));
        while (true)
        {
            //处理任务
            socklen_t len = sizeof(endpoint);
            int sock = accept(_sockfd, (struct sockaddr *)&endpoint, &len); //这个sock就是用来和客户端进行通信的，
            if (sock < 0)
            {
                perror("accept");
                continue;
                ;
            }
            string cli_ip = inet_ntoa(endpoint.sin_addr);
            string ip(cli_ip);
            int port=ntohs(endpoint.sin_port);
            cli_ip += ":";
            cli_ip += to_string(ntohs(endpoint.sin_port));
            cout << "get a link" << cli_ip << endl;
            pthread_t tid;
            //但是调用server函数的时候我们需要的是socket ip port，创建线程的时候只能传一个参数，所以我们需要定义一个结构体存储这些信息
            Param *p=new Param(sock,ip,port);//这样就把数据都初始化传进去了，三个数据都有了

            //对于主线程accept上来的文件描述符，不能由主线程来close，应该让操作这个sockfd的线程进行关闭
            //同样，新线程也不能对监听sockfd进行关闭
            pthread_create(&tid, nullptr, Routine, (void *)p); //在这里执行操作，所以这个地方就要传入套接字

            /*
                各个线程共享一个文件描述符表，因此当主线程调用accept的时候，其他创建的新线程是可以直接访问到这个文件描述符的
                由于此时新线程并不知道它要服务的客户端所要对应的是哪一个文件描述符，所以此时主线程要告诉新线程应该要去访问的文件描述符的值
                就是要操作哪一个套接字

            */
        }
    }
};