#pragma once
#include<iostream>
#include<string>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<cstdlib>
#include<arpa/inet.h>
#include<cstring>
#include"IO.hpp"
using namespace std;



class TcpClient
{
    private:
    string _ip;//服务器的ip地址
    int _port;//这里的端口就是服务器的端口
    int _sockfd;//创建socket连接到服务器上面
    public:
    TcpClient(string ip="127.0.0.1",int port=8081)
    :_ip(ip),_port(port)
    {}    
    ~TcpClient()
    {
        if(_sockfd>0)
        {
            close(_sockfd);
        }
    }
    void InitClient()
    {
        _sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(_sockfd<0)
        {
            perror("socket");
            exit(1);
        }
        struct sockaddr_in svr;//连接到远程服务器上
        svr.sin_port=htons(_port);
        svr.sin_family=AF_INET;
        svr.sin_addr.s_addr=inet_addr(_ip.c_str());//将string转化成为网络字节序
        socklen_t len=sizeof(svr);
        if(connect(_sockfd,(struct sockaddr*)&svr,len)!=0)
        {
            perror("connect");
            exit(1);
        }


    }

    
    void StartClient()
    {
        
      
        while(true)
        {
            string msg;
            getline(cin,msg);
            //这样发送会出现丢包问题,所以我们要进行修改
            sendmsg(_sockfd,msg.c_str(),msg.size());//将msg的数据和大小都发送过去
            
            //发送完之后就要接收数据
            char* buf;//用来接收数据
            int ss=RecvMsg(_sockfd,&buf);      
            if(ss>0)
            {
                buf[ss]=0;
                cout<<"server echo "<<buf<<endl;
            }
            
            //发送数据



        }
    }
};