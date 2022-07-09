#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include<cstdio>
using namespace std;

int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        cout<<"port"<<endl;
        return 1;
    }
    // 1.创建套接字，打开网络文件，作为一个服务器，要让客户知道对应的服务器的地址，不然就不会访问了
    //服务器的socket消息，必须得被客户知道，一般的服务器的port，必须是众所周知的（人，app，），而且轻易不能被改变，
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        cerr << "socket create error" << endl;
        return 1;
    }
    // 2.要给服务器绑定服务器的ip和端口号
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(atoi(argv[1])); //这里面使用的都必须是主机序列
    //云服务器不能允许用户直接绑定公网ip，实际正常写的时候，也不会直接指明ip
    // local.sin_addr.s_addr=inet_addr("127.0.0.1");//将字符串序列的点分十进制的风格转化为主机序列
    local.sin_addr.s_addr = INADDR_ANY; //如果bind是确定的ip（主机），意味着，只有发到该ip主机上的数据，才会交给你的网络进程，但是，一般服务器可能有多张网卡，配置多个ip，我们需要的是所有发送到该主机
    //该端口的数据，我不关心这个数据是从哪个ip上的，只要绑定我这个端口，全部数据我都要，一般我们都是用这个
    if (bind(sockfd, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        cerr << "bind cerror " << errno << endl; //把错误码也加上
        return 2;
    }
    // UDP服务器就写完了
    // 3.接下来就是提供服务
    bool quit = false;
    while (!quit)
    {
        //和普通文件的读取有差别
        //在udp里面读取数据就要使用recvfrom
        char buf[1024];
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        //注意，我们在同行的时候双方是互相发送字符串的，（但是对于文件来说，\0是c，c++的标准），不认为发送字符串
        ssize_t s = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&peer, &len); //这里面会返回客户端的消息,-1是为了把它当作字符串看待，
        if (s > 0)
        {
            //在网络通信中，只有报文大小，或者是字节流中字节的个数，没有c/c++字符串这样的概念，（后续我们可以自己处理）
            buf[s] = 0;
            cout << "client # " << buf << endl;
            //我们接收的可能是一个命令，服务器执行这个命令，在客户端上面跑
            FILE* fp=popen(buf,"r");//把这个命令传进去,我们实现了一个命令行解释器
            //读文件
            char t[1024];
            string ech;
            while(fgets(t,sizeof(t),fp))//读到空就结束
            {
                ech+=t;//里面就是各种内容
            }
            pclose(fp);

            ech += ".....";
            //如果我写的udp无法通信，云服务器开放服务，首先需要开放端口，默认的云平台是没有开发特定的端口的，需要所有者，再网络后开放端口
            sendto(sockfd, ech.c_str(), ech.size(), 0, (struct sockaddr *)&peer, len);
        }
        else
        {
            return 1;
        }
    }
    return 0;
}