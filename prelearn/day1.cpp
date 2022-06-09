/*
    套接字的本质上也是一个文件描述符,指向的是一个"网络文件",普通文件的文件缓冲区对应的是磁盘,数据先写入文件缓冲区,再刷新到磁盘
    "网络文件对应的是网卡",他会把文件缓冲区的数据刷新到网卡里面,再发送到网络中

    创建一个套接字做的工作就是打开一个文件,接下来就是要将文件和网络关联起来,这就是绑定的操作,文件缓冲区的数据才知道往哪里刷新 

*/
#define DEFAULT 8081    
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
using namespace std;

class UdpServer
{
    public:
    UdpServer(int port=DEFAULT)//默认端口是8081
    :_port(port)
    ,_sockfd(-1)
    {}
    ~UdpServer()
    {
        if(_sockfd>=0)
        {
            close(_sockfd);
        }
    }
    private:
    int _port;//端口
    int _sockfd;//
};
/*
    首先先要对服务器进行初始化
    1.创建套接字用socket这个接口进行创建\
    int socket(int domain,int type,int protocol);
    
    参数
    * domain:协议家族,我们用的一般都是ipv4,这里会填写AF_INET
    * type:协议类型,可以选择SOCK_DGRAM(数据报,UDP)和SOCK_STREAM(流式服务,TCP)
    * protocol:协议类别,这里填0,可以根据前面的参数自动推导我们需要的类型

*/


void UdpServerInit()
{
    _sockfd=socket()
}