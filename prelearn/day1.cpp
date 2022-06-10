/*
    套接字的本质上也是一个文件描述符,指向的是一个"网络文件",普通文件的文件缓冲区对应的是磁盘,数据先写入文件缓冲区,再刷新到磁盘
    "网络文件对应的是网卡",他会把文件缓冲区的数据刷新到网卡里面,再发送到网络中

    创建一个套接字做的工作就是打开一个文件,接下来就是要将文件和网络关联起来,这就是绑定的操作,文件缓冲区的数据才知道往哪里刷新 


    socket(int domain,int type,protocol)
    domain就是ip协议是什么，有AF_INET,和AF_INET6
    type就是创建套接字选择数据传输协议，流式协议SOCK_STREAM(TCP),报式协议SOCK_DRARAM(UDP)
    protocol 用0就可以了，帮我们直接推导出需要的东西

    返回值，成功返回一个新套接字所对应的文件描述符，失败返回-1

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

    2.bind
    给socket绑定一个ip+端口号（地址结构）
    int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
    addrlen是地址结构的字节大小
    返回值，成功返回0，失败error

    3.listen
    设置同时和服务器建立连接的上限数（同时进行3次握手的客户端数量）
    int listen(int sockfd,int backlog)
    backlog可以建立的最大上限数，最大值是128

    4.accept
    阻塞等待客户端建立连接，成功的话，返回一个和客户端成功连接的socket文件描述符
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    * sockfd：把一个socket传进来主要是用他的ip地址和端口来和客户端去建立连接
    * addr这里是一个传出参数，成功于服务器建立连接的那个客户端的地址结构（ip+port）
     
    socklen_t clit_addrlen=sizeof(addr);
    * addrlen是一个传入传出的参数，入：addr的大小，出来客户端addr的实际大小，&clit_addrlen
    
    返回值： 成功：能于服务器进行数据通信的socket文件描述符
             失败：返回-1
    所以服务器这一端有两个socket


    5.conncet
    int connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
    使用现有的socket和服务器进行一个连接
    * addr这里是服务器的地址结构，因为要和服务器进行连接，所以肯定要获取服务器的结构
    * addrlen就是服务器地质结构的大小

    返回值
    如果连接绑定成功就返回0，失败返回-1
    

    客户端的ip地址和端口号是系统自己干的，“隐式绑定”

*/
/*

server:
    TCP 通信流程
    1.socket（）   创建一个socket
    2.bind（）绑定端口号，ip地址
    3.listen   设置监听上限
    4.accept   阻塞监听客户端连接
    5.read(fd)     读取客户端的连接
    6.write（fd）    
    7close()

    client
    1. socket()
    2.connct   于服务器建立连续
    3. srite写数据到socket
    4.read   读转化后的数据
    5. 显示读取结果

*/



void UdpServerInit()
{
    _sockfd=socket()
}