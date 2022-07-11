#pragma once
#include <iostream>
#include <pthread.h>
#include"unistd.h"
#include<cstring>
using namespace std;

namespace ns_task
{
    class Task
    {
    private:
        int _sockfd;

    public:
        Task() //无参构造，为了拿任务，不需要参数列表
            : _sockfd(-1)
        {
        }
        //进行函数重载
        Task(int sockfd)
            : _sockfd(sockfd)
        {
        }

        ~Task()
        {
        }
        int Run()//执行任务
        {
            while (true)
            {
                //因为TCP 是面向字节流的,就如同文件一样,就可以进行正常的读写
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                ssize_t s = read(_sockfd, buf, sizeof(buf) - 1);
                if (s > 0)
                {
                    buf[s] = 0;
                    //任务
                    //1. 这样我们判断一下，如果是一行命令，那么就执行这个命令，返回给用户
                    //2. 如果这个是一个单词，就把这个单词转为中文，我们可以使用哈希，如果有一个大的词典，就可以进行匹配
                    //3. 如果这些都不是的话，就原封不动的返回给用户

                    cout << "client # " << buf << endl;
                    string echo = "server send ";
                    echo += buf;
                    write(_sockfd, echo.c_str(), echo.size());
                }
                else if (s == 0)
                {
                    cout << "client quit" << endl; //客户端ctrl c之后就断开了连接
                    close(_sockfd);
                    break;
                }
                else
                {
                    cerr << "error" << endl;
                    close(_sockfd);
                    break;
                }
            }
            // close(_sockfd);//跑完了就把套接字关掉就可以了
        }

        int operator()() //重载一个函数
        {
            return Run();
        }
    };
}