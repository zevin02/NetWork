#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include<cstdlib>
#include<signal.h>
using namespace std;

void serverio(int newsockfd)
{
    while (true)
    {
        //因为TCP 是面向字节流的,就如同文件一样,就可以进行正常的读写
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        ssize_t s = read(newsockfd, buf, sizeof(buf) - 1);
        if (s > 0)
        {
            buf[s] = 0;
            cout << "client # " << buf << endl;
            string echo = "server send ";
            echo += buf;
            write(newsockfd, echo.c_str(), echo.size());
        }
        else if (s == 0)
        {
            cout << "client quit" << endl; //客户端ctrl c之后就断开了连接
            break;
        }
        else
        {
            cerr << "error" << endl;
            break;
        }
    }
}
int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cerr << "socket error" << endl;
        return 1;
    }
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(8080);
    local.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        cerr << "bind error" << endl;
        return 2;
    }
    // 3.因为tcp是面向连接的,在通信的时候要建立连接,
    // a在通信钱,需要建立连接,b:然后才能通信
    //一般客户端来建立连接,服务器是被动接收连接
    //我们当前写的是一个server,周而复始的不间断的等待客户到来
    //我们要不断的给用户提供一个建立连接的功能
    //设置套接字为listen状态
    if (listen(sockfd, 5) < 0) //设置为被连接状态,这样别人就可以连接到我了
    {
        cerr << "listen error" << endl;
    }
    signal(SIGCHLD,SIG_IGN);//子进程退出的时候会给父进程发送信号，这里我们去忽略一下子进程发送的信号,子进程会自动退出释放资源
    while (1)
    {
        struct sockaddr_in peer;
        memset(&peer, 0, sizeof(peer));
        socklen_t len = sizeof(peer);
        int newsockfd = accept(sockfd, (struct sockaddr *)&peer, &len);
        if (newsockfd < 0)
        {
            continue; //连接失败就继续连接
        }
        //我们这里现在是一个单进程，一次只能允许一个人使用
        //我们使用多进程来操作一下

        pid_t id = fork();
        if (id < 0)
        {
            continue;
        }
        else if (id == 0)
        {
            // child
            serverio(newsockfd);
            exit(1);//子进程执行完了，父进程就要等
            //子进程和父进程的文件描述符是共享的，退出后要关闭文件描述符
        }
        else
        {
            //do nothing ,因为父进程忽略了，所以就不需要去wait，父进程不断的建立连接，子进程不断的提供服务
        }
        //提供服务
    }
    return 0;
}