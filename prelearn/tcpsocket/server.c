#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<ctype.h>
#define SERVER_PORT 9527
int main()
{
    //1.先socket
    int sockfd=socket(AF_INET,SOCK_STREAM,0);//这个是后期和服务器建立连接的
    if(sockfd<0)
    {
        perror("socket");
        
    }
    //2.bind
    struct sockaddr_in s;
    s.sin_family=AF_INET;
    s.sin_port=htons(SERVER_PORT);//需要将本地端口字节序转化成网络字节序
    s.sin_addr.s_addr=htonl(INADDR_ANY);//这里面就是我们需要的ip地址,将他转化成为网络字节序
    int d=bind(sockfd,(struct sockaddr*)&s,sizeof(s));
    if(d!=0)
    {
        perror("bind");
        exit(1);
    }
    listen(sockfd,128);//进行监听
    struct sockaddr_in clie;//监听的客户端的ip和端口
    socklen_t addrlen=sizeof(clie);//客户端地址结构的长度
    int cliefd=accept(sockfd,(struct sockaddr*)&clie,&addrlen);//阻塞监听和客户端建立连接,第二个参数是传入传出
    //accept调用完了就可以在这里等待客户端发送数据进来
    //这个文件描述符是用来进行数据通信的,
    printf("-1");
    
    if(cliefd<0)
    {
        perror("accept");
        exit(1);
    }
    char dst[1024];
    //传回来的都是网络字节序
    printf("client ip:%s port:%d\n",inet_ntop(AF_INET
    ,&clie.sin_addr.s_addr,dst,sizeof(dst))
    ,ntohs(clie.sin_port));//inet_ntop获得的就是string类型的ip地址,这个sin_port拿出来的都是网络字节序的，所以我们需要转化成主机的
    //成功之后服务器就该读了
    //把数据从客户端里面读出来
    while(1)
    {
    char str[100];
    int len=read(cliefd,str,sizeof(str));
    // if(len>0)
    // {
    //     str[len]='\0';
    // }
    //读出来之后要进行转换
    // printf("len=%d ",len);
    write(1,str,len);//
    int i=0;
    for(i=0;i<len;i++)
    {
        toupper(str[i]);
    }
    //转换完之后就要写回去
    write(cliefd,str,len);
    }
    close(cliefd);
    close(sockfd);

    return 0;
}