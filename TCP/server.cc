#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include<cstdlib>
#include<signal.h>
#include<string>
#include<sys/wait.h>
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
    // signal(SIGCHLD,SIG_IGN);//子进程退出的时候会给父进程发送信号，这里我们去忽略一下子进程发送的信号,子进程会自动退出释放资源
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
        uint16_t cli_port=ntohs(peer.sin_port);//保证代码的可移植性
        string ip=inet_ntoa(peer.sin_addr);//
        cout<<"get a link -> : "<<cli_port<<" : "<<newsockfd<<" "<<ip<<endl;//我们发现每一个都是4
        pid_t id = fork();
        if (id < 0)
        {
            continue;
        }
        else if (id == 0)
        //曾经被父进程打开的fd，是否会被子进程继承？无论父子进程的哪一个，强烈建议关闭不需要的fd，监听描述符也会被子进程看到，
        //我们在子进程这边把监听描述符关闭
        {
            // child
            close(sockfd);
            if(fork()>0)
            exit(0);//这个叫做退出的是子进程，向后走的进程是孙子进程

            //因为父进程一进来就挂掉了，所以孙子进程就变成了孤儿进程，后会被操作系统给回收掉，
            serverio(newsockfd);
            //如果不关闭文件描述符，就会导致文件描述符泄露的问题
            close(newsockfd);
            exit(1);//子进程执行完了，父进程就要等
            //子进程和父进程的文件描述符是共享的，退出后要关闭文件描述符
        }
        else
        {
            //do nothing ,因为父进程忽略了，所以就不需要去wait，父进程不断的建立连接，子进程不断的提供服务
            //阻塞等待不需要等待，耗时间很大，所以使用忽略sigchild信号，
            waitpid(id,nullptr,0);//这里等待的时候会不被阻塞等待太长时间，因为它子进程刚一创建出来就退出了孙子进程执行完之后就被OS给回收了
            close(newsockfd);//fork已经返回了
        }
        //提供服务
    }
    return 0;
}