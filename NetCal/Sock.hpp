#include <iostream>
#include <unistd.h>
using namespace std;
#include<sys/socket.h>
#include<sys/types.h>
#include<cstdlib>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<cstring>
#include<string>
//这些静态的套接字都是属于类而不属于对象
class Sock
{
public:
    static int  Socket()
    {
        int sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(sockfd<0)
        {
            perror("socket");
            exit(2);
        }
        return sockfd;
    }
    static void Listen(int sockfd)
    {
        if(listen(sockfd,5)<0)
        {
            perror("listen");
            exit(4);
        }
    }
    static void Bind(int sockfd,uint16_t port)
    {
        struct sockaddr_in local;
        memset(&local,0,sizeof(local));
        local.sin_family=AF_INET;
        local.sin_addr.s_addr=INADDR_ANY;
        local.sin_port=htons(port);
        if(bind(sockfd,(struct sockaddr*)&local,sizeof(local))<0)
        {
            perror("bind");
            exit(3);
        }
    }
    static int Accept(int sockfd)
    {
        struct sockaddr_in peer;
        bzero(&peer,0);
        socklen_t len=sizeof(peer);
        int fd=accept(sockfd,(struct sockaddr*)&peer,&len);
        if(fd<0)
        {
            perror("accept");
            exit(5);
        }
        return fd;
    }
    static void Connect(int sockfd,string ip,int port)
    {
        struct sockaddr_in svr;
        memset(&svr,0,sizeof(svr));
        svr.sin_family=AF_INET;
        svr.sin_port=htons(port);
        svr.sin_addr.s_addr=inet_addr(ip.c_str());

        if(connect(sockfd,(struct sockaddr*)&svr,sizeof(svr))<0)
        {
            perror("connect");
            exit(6);
        }
    }

};