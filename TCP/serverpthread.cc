#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <string>
#include <sys/wait.h>
using namespace std;
#include <pthread.h>
#include "threadpool.hpp"
#include "Task.hpp"
using namespace ns_task;
using namespace ns_threadpool;

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
        uint16_t cli_port = ntohs(peer.sin_port);                                          //保证代码的可移植性
        string ip = inet_ntoa(peer.sin_addr);                                              //
        cout << "get a link -> : " << cli_port << " : " << newsockfd << " " << ip << endl; //我们发现每一个都是4
        //先构建一个任务
        Task t(newsockfd);
        // 2.将任务push到后端的线程池就可以了
        ThreadPool<Task>::GetInstance()->PushTask(t); //这样就完了//同样这是懒汉单例模式

        //这样使用的话，线程是不会增多的，退出的时候，线程池也不会再变化，这就不会有线程问题

        
    }
    return 0;
}
//但是这样去写有两个问题
// 1，创建进程和线程没有上限，进程和线程越多，进程切换就有消耗，时间也就更久了，系统运行的也就特别的慢了，当客户连接来了才给客户创建进程/线程
//进程或者线程池版本，就可以直接用之前的线程池用进去,这里我们结合单例模式