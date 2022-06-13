#include<iostream>
#include<string>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<cstdlib>
#include<arpa/inet.h>
using namespace std;



class TcpClient
{
    private:
    string _ip;
    int _port;
    int _sockfd;
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
        string msg;
        struct sockaddr_in cli;
        cli.sin_family=AF_INET;
        cli.sin_port=htons(_port);
        cli.sin_addr.s_addr=inet_addr(_ip.c_str());
        socklen_t len=sizeof(cli);
        while(true)
        {
            getline(cin,msg);

            sendto(_sockfd,msg.c_str(),msg.size(),0,(struct sockaddr*)&cli,len);
            //发送过去了之后
            char buf[1024];

            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            ssize_t ss=recvfrom(_sockfd,buf,sizeof(buf)-1,0,(struct sockaddr*)&peer,&len);
            if(ss>0)
            {
                buf[ss]=0;
                cout<<"server echo "<<buf<<endl;
            }
            

        }
    }
};