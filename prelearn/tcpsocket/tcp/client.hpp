#include<iostream>
#include<string>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<cstdlib>
#include<arpa/inet.h>
#include<cstring>
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

    int writen(const char* msg,int size)
    {
        const char* buf=msg;//指向它的地址，发送出去
        int count=size;//剩余需要发送的字节数
        while(count>0)
        {
            int len=send(_sockfd,buf,count,0);
            if(len==-1)
            {
                //发送失败
                return -1;
            }
            else if(len==0)
            {
                continue;//没发送出去，再发送一次
            }
            else
            {
                //发送成功
                buf+=len;
                count-=len;
            }
        }
        return size;//发送成功，发送完成

    }


    void sendmsg(const char* msg,int len)
    {
        //先申请包头
        char* data=(char*)malloc(sizeof(char)*(len+4));//多加的4是为了数据的长度
        int biglen=htonl(len);
        memcpy(data,&biglen,4);//拷贝4个字节过去
        memcpy(data+4,msg,len);//拷贝len个长度过去
        int ret=writen(data,len+4);//真正的传输数据
        if(ret==-1)
        {
            //发送失败
            free(data);//把data的内存销毁掉
            close(_sockfd);
            
        }
        else
        free(data);

    }
    void StartClient()
    {
        
        struct sockaddr_in cli;
        cli.sin_family=AF_INET;
        cli.sin_port=htons(_port);
        cli.sin_addr.s_addr=inet_addr(_ip.c_str());
        socklen_t len=sizeof(cli);
        while(true)
        {
            string msg;
            getline(cin,msg);
            //这样发送会出现丢包问题,所以我们要进行修改
            sendmsg(msg.c_str(),msg.size());//将msg的数据和大小都发送过去
            
            // sendto(_sockfd,msg.c_str(),msg.size(),0,(struct sockaddr*)&cli,len);
            // //发送过去了之后
            char buf[1024];

            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            ssize_t ss=recvfrom(_sockfd,buf,sizeof(buf)-1,0,(struct sockaddr*)&peer,&len);
            if(ss>0)
            {
                buf[ss]=0;
                cout<<"server echo "<<buf<<endl;
            }
            
            //发送数据



        }
    }
};