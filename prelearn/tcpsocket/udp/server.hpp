#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <netinet/in.h>
#define DEFAULT_PORT 8081
using namespace std;
class UDPSERVER
{
private:
    int _sockfd; //服务器
    int _port;   //端口号
public:
    UDPSERVER(int port = DEFAULT_PORT)
        : _sockfd(-1) //先初始化为-1
          ,
          _port(port)
    {
    }

    void UDPINIT()
    {
        //创建套接字
        //协议家族IPV4.类型为报文，0推导类型
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0)
        {
            perror("socket");
            exit(-1);
        }
        cout << "socket create successfully,sockfd=" << _sockfd << endl;
        //创建成功就继续绑定端口号和ip地址，创建失败的话就退出进程
        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = INADDR_ANY; //表示0，表示取消对单个ip的绑定，服务器有多个IP，如果指明了绑定哪一个ip，那么服务端就只能从这个ip获取数据
        //如果用这个的话，那么服务端可以接受来自本主机任意ip对该端口号发送出来的数据
        if (bind(_sockfd, (struct sockaddr *)&local, sizeof(local)) == -1)
        {
            perror("bind");
            exit(-1);
        }
        cout << "bind success port:" << _port << endl;
    }
    void UDPSTART()
    {
        /*
        ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr* src_addr, socklen_t *addrlen);
        从一个套接字中获取信息，面向无连接
        * sockfd:从该套接字获取信息
        * buf： 把数据读取到这个缓冲区里面
        * len ： 一次读多少自己的数据
        * flags : 表示阻塞读取
        * src_addr:一个输出型参数，获取到对端的信息有端口号，ip地址，方便后续我们对其进行响应
        * addlen: 输入输出性参数，传入一个想要读取对端src_addr的长度，最后冯大辉实际读取的长度
        *
        返回值： 实际读取到的数据大小


        ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

        * sockfd:把数据写入到该套接字里面
        * buf向该缓冲区里面发送
        * len发送多少数据
        * flags表示阻塞发送
        * dest_addr本地的网络相关属性，填充号了发送给对方，确保对方能响应
        * addrlen addr 的实际大小
        *
        成功返回实际写入的数据大小，失败返回-1


        */

        while (true)
        {
            string peerip;
            int peerport;
            char buf[1024];                                                                            //这里面存的就是远端发送的数据
            struct sockaddr_in peer;                                                                   //获取远端的数据和信息，ip和端口号
            socklen_t len = sizeof(peer);                                                              //输入输出型参数
            ssize_t size = recvfrom(_sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&peer, &len); // flag 0表示阻塞等待
                                                                                                       //获得到了客户端发送来的退出请求，客户端直接退出，服务器继续运行
            peerip = inet_ntoa(peer.sin_addr);                                                         //返回值是char*,将网络字节序转化为主机序列中的ip
            if (strcmp("exit", buf) == 0)
                continue;
            if (size > 0)
            {
                buf[size] = 0; // size为实际读取到的数据，
                //获取远端的端口
                //现在传过来的都还是网络字节序
                peerport = ntohs(peer.sin_port);

                cout << peerip << ":" << peerport << "# " << buf << endl;
                string echo_msg = "server get->";
                echo_msg += buf;
                //发送数据
                sendto(_sockfd, echo_msg.c_str(), echo_msg.size(), 0, (struct sockaddr *)&peer, len);
            }
            else
            {
                cerr << "recvfrom error" << endl;
                string error_msg = "server recvfrom error";
                sendto(_sockfd, error_msg.c_str(), error_msg.size(), 0, (struct sockaddr *)&peer, len);
            }
        }
    }

    ~UDPSERVER()
    {
        if (_sockfd >= 0)
        {
            //使用完之后就要关掉
            close(_sockfd);
        }
    }
};

//客户端发出退出请求，只需要让客户端退出即可，服务端继续去读取其他客户端的请求即可，不能因为一个客户端退出，服务端就退出
//服务端一次读取数据失败也不可以直接退出，重新读取就可以了

