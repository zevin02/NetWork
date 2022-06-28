#pragma once
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<cstdlib>
#include<arpa/inet.h>
#include<cstring>
#include<string>
using namespace std;



// class Sock
// {
//     public:
//     static int Socket()
//     {
//         int sockfd=socket(AF_INET,SOCK_STREAM,0);
//         if(sockfd<0)
//         {
//             perror("socket");
//             exit(1);
//         }
//         return sockfd;
//     }
//     static void Listen(int sockfd)
//     {
//         if(listen(sockfd,5)<0)
//         {
//             perror("listen");
//             exit(1);
//         }
//     }
//     static void Bind(int sockfd,int port,string ip)
//     {
//         struct sockaddr_in local;
//         memset(&local,0,sizeof(local));
//         local.sin_family=AF_INET;
//         local.sin_port=htons(port);
//         local.sin_addr.s_addr=inet_addr(ip.c_str());

//         if(bind(sockfd,(struct sockaddr*)&local,sizeof(local))<0)
//         {
//             perror("bind");
//             exit(1);
//         }
//     }
//     static void SetSockOpt(int sockfd)
//     {
//         //端口复用
//         int opt=1;
//         setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(opt));
//     }

//     static int Accept(int sockfd)
//     {
//         struct sockaddr_in peer;
//         memset(&peer,0,sizeof(peer));
//         socklen_t len=sizeof(peer);
//         int peerfd=accept(sockfd,(struct sockaddr*)&peer,&len);
//         if(peerfd<0)
//         {
//             perror("accept");
//             exit(2);
//         }
//         return peerfd;
//     }
// };


#pragma once
#include<iostream>
using namespace std;
#include<sys/types.h>
#include<cstdio>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<cstdlib>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<unistd.h>
class Sock
{
  public:
    static int  Socket()
    {
      int sock = socket(AF_INET,SOCK_STREAM,0);
      if(sock < 0)
      {
        perror("sock");
        exit(2);
      }
      return sock;
    }

    static void Bind(int sockfd,int port)
    {
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY; 
      addr.sin_port = htons(port);

      
      socklen_t len = sizeof(addr);
      if(bind(sockfd,(struct sockaddr*)&(addr),len) < 0)
      {
        perror("bind");
        exit(3);
      }
    }

    static void Listen(int sockfd)
    {
     if(listen(sockfd,5) < 0)
     {
       perror("Listen");
       exit(4);
     }
    }

    static int Accept(int sockfd)
    {
     struct sockaddr_in peer;
     socklen_t len = sizeof(peer);
     int fd = accept(sockfd,(struct sockaddr*)&peer,(socklen_t*)&len); 
     if( fd < 0 )
     {
       perror("accept");
     }
     return fd;
    }
    static void Setsockopt(int sockfd)
    {
      int opt = 1;
      setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    }

};
