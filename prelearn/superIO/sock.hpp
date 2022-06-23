#pragma once
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<cstdlib>
#include<arpa/inet.h>
using namespace std;


class Sock

{
    public:
    static int Socket()
    {
        int sock=socket(AF_INET,SOCK_STREAM,0);
        if(sock<0)
        {
            perror("socket");
            exit(1);
        }
        return sock;
    }
    static void Bind(int sockfd,int port)
    {
        struct sockaddr_in addr;
        addr.sin_family=AF_INET;
        addr.sin_port=htons(port);
        addr.sin_addr.s_addr=INADDR_ANY;
        socklen_t len=sizeof(addr);
        if(bind(sockfd,(struct sockaddr*)&addr,len)<0)
        {
            perror("bind");
            exit(1);
        }

    }
    static void Listen(int sockfd)
    {
        if(listen(sockfd,5)<0)
        {
            perror("listen");
            exit(1);
        }
    }
    static int Accept(int sockfd)
    {
        struct sockaddr_in peer;
        socklen_t len=sizeof(peer);
        int fd=accept(sockfd,(struct sockaddr*)&peer,&len);
        if(fd<0)
        {
            perror("accept");
        }
        return fd;
    }
    static void Setsockopt(int sockfd)
    {
        int opt=1;
        setsockopt()
    }

};