#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <cstring>
#include <arpa/inet.h>
#include <string>
#include<pthread.h>
#include<cstdlib>
using namespace std;
#define MAX 1024
#define DEFUALTPORT 8081
#define BACKLOG 20
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
    static void* Routine(void * args)//因为在类里面，所以不要this指针
    {
        int sock=*(int*)args;
        delete args;
        pthread_detach(pthread_self());//线程分离就不需要再去join了
        cout<<"create a new pthread for IO"<<endl;
        char buff[MAX];
        while(true)
        {
            //读取数据
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            ssize_t s=recvfrom(sock,buff,sizeof(buff)-1,0,(struct sockaddr*)&peer,&len);//peer里面就是远程的数据了
            if(s>0)
            {
                buff[s-1]=0;
                fflush(stdout);
                cout<<"#client:"<<buff<<endl;
                string msg="#client";
                msg+=buff;
                sendto(sock,msg.c_str(),msg.size(),0,(struct sockaddr*)&peer,len);

            }
            else
            {
                cout<<"error data"<<endl;
                break;
            }
        }
    }
    void StartTcp()
    {
        sockaddr_in endpoint;
        while (true)
        {
            //处理任务
            socklen_t len = sizeof(endpoint);
            int sock = accept(_sockfd, (struct sockaddr *)&endpoint, &len);//这个sock就是用来和客户端进行通信的，
            if (sock < 0)
            {
                perror("accept");
                continue;
                ;
            }
            string cli_ip = inet_ntoa(endpoint.sin_addr);
            cli_ip += ":";
            cli_ip += to_string(ntohs(endpoint.sin_port));
            cout << "get a link" << cli_ip << endl;
            int * p=new int(sock);
            pthread_t tid;
            pthread_create(&tid,nullptr,Routine,(void*)p);//在这里执行操作

        }
    }
};