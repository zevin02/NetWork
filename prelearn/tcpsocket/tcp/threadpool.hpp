#pragma once
#include <iostream>
#include <string>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include"IO.hpp"
#include <cstring>
using namespace std;
//设计一个handler类，在handler类里面对（）操作符进行重载，将（）操作符的执行动作重载为执行server函数的代码
class Handler
{
public:
    Handler()
    {
    }
    ~Handler()
    {
    }


    void operator()(int sock, string cliip, int cliport)
    {
        //执行server函数的代码
        // char buff[1024];
        // while (true)
        // {
        //     ssize_t size = read(sock, buff, sizeof(buff) - 1);
        //     if (size > 0) //读取成功
        //     {
        //         buff[size] = '\0';
        //         cout << cliip << ":" << cliport << "#" << buff << endl;
        //     }
        //     else if (size == 0) //对端关闭了
        //     {
        //         cout << cliip << ":" << cliip << " close!" << endl;
        //     }
        //     else
        //     {
        //         //读取失败
        //         cerr << sock << " read error!" << endl;
        //         break; //读取失败的化就关闭
        //     }
        // }

        char *buff; //接收数据
        while (true)
        {
            //读取数据
            // struct sockaddr_in peer;
            // socklen_t len = sizeof(peer);
            // ssize_t s = recvfrom(sock, buff, sizeof(buff) - 1, 0, (struct sockaddr *)&peer, &len); // peer里面就是远程的数据了

            //使用新的解决粘包问题的读取数据的方法
            int s = RecvMsg(sock, &buff); //读取数据
            if (s > 0)
            {
                buff[s] = 0;
                fflush(stdout);
                cout << "#client:" << buff << endl;
                string msg = "#client ";
                msg += buff;
                //发送回客户端
                sendmsg(sock, msg.c_str(), msg.size()); //发送数据
                // sendto(sock, msg.c_str(), msg.size(), 0, (struct sockaddr *)&peer, len);
            }
            else
            {
                cout << "error data" << endl;
                break;
            }
        }

        close(sock); //这个线程用完了就要把这个文件描述符关掉
        cout << cliip << ":" << cliport << " service done!" << endl;
    }
};

class Task //任务
{
private:
    int _sockfd;
    string _cliip;
    int _cliport;
    Handler _handler; //处理方法
public:
    Task() //无参数，就是为了线程池取任务出来执行
    {
    }
    Task(int sock, string ip, int port) //构造函数里面放任务
        : _sockfd(sock), _cliip(ip), _cliport(port)
    {
    }
    ~Task()
    {
    }

    //处理任务的函数
    void Run()
    {
        _handler(_sockfd, _cliip, _cliport); //调用仿函数
    }
};

namespace ns_threadpool
{
    const int g_num = 5;
    template <class T>
    class ThreadPool //线程池
    {

    private:
        int num_;                  //一个线程池里面有多少个任务
        std::queue<T> task_queue_; //任务队列,临界资源
        pthread_mutex_t mtx_;
        pthread_cond_t cond_;

    public:
        ThreadPool(int num = g_num) : num_(num)
        {
            pthread_mutex_init(&mtx_, nullptr);
            pthread_cond_init(&cond_, nullptr);
        }

        ~ThreadPool()
        {
            pthread_mutex_destroy(&mtx_);
            pthread_cond_destroy(&cond_);
        }

        //在类中，要让

        static void *Rountine(void *args)
        //也不能访问类里面非static成员
        {

            pthread_detach(pthread_self()); //实现线程分离就不要再去join等待了
            ThreadPool<T> *tp = (ThreadPool<T> *)args;
            while (true)
            {
                //从任务队列里面去拿一个任务
                //执行任务，要先把这个任务队列锁主

                //每个线程他跟放任务的线程一样，都是竞争式的去拿一个任务
                tp->Lock();
                //先检测任务队列是否有一个任务
                while (tp->IsEmpty())
                {
                    //检测到任务队列为空
                    //此时线程就挂起等待
                    tp->Wait();
                }
                //该任务队列里面一定有任务了
                T t;
                tp->PopTask(&t);
                //任务就拿到了
                tp->UnLock();
                t.Run(); //可能有多个线程在处理任务,

                sleep(1);
            }
        }
        void InitThreadPool()
        {
            //初始化一批线程，
            //这样就不要每次用都要去开辟线程了
            pthread_t tid; //一次创建一批线程
            for (int i = 0; i < num_; i++)
            {
                pthread_create(&tid, nullptr, Rountine, (void *)this);
                //在类中不能执行线程的方法，因为他都有隐藏的this指针
                //所以我们需要使用静态的函数，就没有了this指针
            }
        }
        void PopTask(T *out)
        {
            *out = task_queue_.front();
            task_queue_.pop();
        }
        void Wait()
        {
            pthread_cond_wait(&cond_, &mtx_);
        }
        bool IsEmpty()
        {
            return task_queue_.empty();
        }
        void Lock()
        {
            pthread_mutex_lock(&mtx_);
        }
        void UnLock()
        {
            pthread_mutex_unlock(&mtx_);
        }
        void Wakeup()
        {
            pthread_cond_signal(&cond_);
        }
        void PushTask(const T &in)
        {
            //塞任务，就相当于一个生产者，生产者之间要进行互斥访问
            Lock();
            task_queue_.push(in);
            UnLock();
            Wakeup();
        }

        //万一任务队列里面一个任务都没有的话，那么线程池里面的每一个线程就要处于休眠状态，挂起等待
    };
}