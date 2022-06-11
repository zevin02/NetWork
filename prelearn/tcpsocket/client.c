#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
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
    cli.sin_port=htons(9527);
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
        write(sockfd,"FFFF",4);
        sleep(1);
        char buffer[1024];
        ssize_t ret=read(sockfd,buffer,sizeof(buffer));
        write(1,buffer,ret);
    }
    close(sockfd);
    return 0;
}