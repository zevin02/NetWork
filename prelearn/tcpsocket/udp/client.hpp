#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
using namespace std;

class UDPCLIENT
{
private:
    int _port;
    int _sockfd;
    string _ip;

public:
    UDPCLIENT(int port=8080, string ip="127.0.0.1")
        : _port(port), _sockfd(-1), _ip(ip)
    {
    }
    ~UDPCLIENT()
    {
        if (_sockfd >= 0)
        {
            close(_sockfd);
        }
    }
    void UDPINIT()
    {
        //创建套接字
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0)
        {
            perror("socket");
            exit(-1);
        }
        //不需要绑定端口号，sendto会自动分配一个端口号
        cout << "sockfd creat successfully,sockfd:" << _sockfd << endl;
        
    }
    void UDPSTART()
    {
        //客户端启动发送数据，和读取响应，调用sendto 和recvfrom两个接口，发送数据，需要将自己的网络信息发送给对方
        struct sockaddr_in peer;
        peer.sin_family = AF_INET;
        peer.sin_port = htons(_port);
        char buf[1024];
        // peer.sin_addr.s_addr = inet_pton(AF_INET, _ip.c_str(), buf);
        peer.sin_addr.s_addr=inet_addr(_ip.c_str());
        string msg;
        while (1)
        {
            getline(cin, msg);
            sendto(_sockfd, msg.c_str(), msg.size(), 0, (struct sockaddr *)&peer, sizeof(peer));
            if (msg == "exit")
            {
                break;
            }
            char buf[1024];
            struct sockaddr temp;
            socklen_t len=sizeof(temp);
            ssize_t size=recvfrom(_sockfd,buf,sizeof(buf)-1,0,(struct sockaddr*)&temp,&len);
            if(size>0)
            {
                buf[size]=0;
                cout<<buf<<endl;
            }
        }
    }
};