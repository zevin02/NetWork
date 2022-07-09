#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<cstring>
#include<unistd.h>
using namespace std;


int main()
{
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        cerr<<"socket error"<<endl;
        return 1;
    }
    struct sockaddr_in local;
    memset(&local,0,sizeof(local));
    local.sin_family=AF_INET;
    local.sin_port=htons(8080);
    local.sin_addr.s_addr=INADDR_ANY;
    if(bind(sockfd,(struct sockaddr*)&local,sizeof(local))<0)
    {
        cerr<<"bind error"<<endl;
        return 2;
    }
    //3.因为tcp是面向连接的,在通信的时候要建立连接,
    //a在通信钱,需要建立连接,b:然后才能通信
    //一般客户端来建立连接,服务器是被动接收连接
    //我们当前写的是一个server,周而复始的不间断的等待客户到来
    //我们要不断的给用户提供一个建立连接的功能
    //设置套接字为listen状态
    if(listen(sockfd,5)<0)//设置为被连接状态,这样别人就可以连接到我了
    {
        cerr<<"listen error"<<endl;

    }
    while(1)
    {
        struct sockaddr_in peer;
        memset(&peer,0,sizeof(peer));
        socklen_t len=sizeof(peer);
        int newsockfd=accept(sockfd,(struct sockaddr*)&peer,&len);
        if(newsockfd<0)
        {
            continue;//连接失败就继续连接
        }
        //提供服务
        while(true)
        {
            //因为TCP 是面向字节流的,就如同文件一样,就可以进行正常的读写
            char buf[1024];
            memset(buf,0,sizeof(buf));
            ssize_t s=read(newsockfd,buf,sizeof(buf)-1);
        }
    }
    return 0;
}