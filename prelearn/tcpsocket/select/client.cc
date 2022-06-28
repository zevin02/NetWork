#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include"IO.hpp"
#include<string>
#include<iostream>
using namespace std;
int main()
{
    int sockfd=socket(AF_INET,SOCK_STREAM,0);//通过这个socket往服务器里面写东西
    if(sockfd==-1)
    {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in cli;//初始化服务器的地址结构
    cli.sin_family=AF_INET;
    cli.sin_port=htons(8000);
    inet_pton(AF_INET,"127.0.0.1",&(cli.sin_addr.s_addr));
    // cli->sin_addr.s_addr=htonl(INADDR_ANY);//本地中任意的一个ip地址,要连接到服务器的ip
    int res=connect(sockfd,(struct sockaddr*)&cli,sizeof(cli));//使用现有的socket和服务器进行一个连接，所以这里是要获得服务器的信息
    if(res==-1)
    {
        perror("connect");
        exit(1);
    }
    int cnt=6;
    while(cnt--)
    {
        string s;
        cin>>s;
        sendmsg(sockfd,s.c_str(),s.size());

        sleep(1);
        char* buf;
        int size=RecvMsg(sockfd,&buf);
        if(size>0)
        {
            buf[size]=0;
            cout<<buf<<endl;
        }
        else
        {
            exit(1);
        }
        
    }
    close(sockfd);
    return 0;
}