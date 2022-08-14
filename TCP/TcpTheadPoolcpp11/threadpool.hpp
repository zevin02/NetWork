#pragma once
#include <iostream>
#include <string>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include<mutex>
#include<condition_variable>
#include<thread>
#include<vector>
#include<chrono>
#include<atomic>
namespace ns_threadpool
{
    const int g_num = 5;
    template <class T>
    class ThreadPool //线程池
    {

    private:
        int num_;                  //一个线程池里面有多少个任务
        std::queue<T> task_queue_; //任务队列,临界资源
        mutex mtx_;
        condition_variable cond_;
        static ThreadPool<T> *ins; //静态成员在所有的对象里面只有一个静态成员，必须要通过静态变量来获取对象
        //保存内存的可见性
    private:
        //单例的话，就不能让构造函数暴露在外面，否则，只有有构造函数，就能初始化
        //构造函数必须得实现，当时必须得私有
        ThreadPool(int num = g_num) : num_(num)
        {
           
        }

        ThreadPool(const ThreadPool &tp) = delete;
        ThreadPool(const ThreadPool &&tp) = delete;
        // c++11的新特性
        //静止编译器生成拷贝构造，
        //=delete就是禁止调用这个函数，在私有里面

        ThreadPool operator=(const ThreadPool &tp) = delete;
        ThreadPool operator=(const ThreadPool &&tp) = delete;
        //把赋值运算符也禁止掉，这也就可以避免创建多个对象

    public:
        static ThreadPool<T> *GetInstance() //这个必须是使用静态的，非静态函数都是有对象的，静态函数才是没对象的,没有this指针
        {
            //获得实例
            if (ins == nullptr)//双判定，减少锁的争用，提高单例获取的效率，
            //假如说有的线程进来发现不为空，就可以直接走了，如果同时为nullptr的化，那么再把他们放进来争抢锁资源、

            {

                mutex mtx;//在类里面
                unique_lock<mutex> lock(mtx);

                //当前的单例对象还没有被创建
                if (ins == nullptr)
                //假如是在多线程的情况下，那么多个线程执行的时候，都是nullptr，都创建了对象，那么就出现了线程安全
                {
                    //就创建它
                    ins = new ThreadPool<T>(); //创建一个，使用构造函数
                    //创建出来了一个单例之后，就直接给他初始化一个池就行了
                    ins->InitThreadPool();
                    std::cout << "首次加载对象" << std::endl;
                }
            }
            return ins;
        }

        ~ThreadPool()
        {
            
        }

        //在类中，要让

        static void *Rountine(ThreadPool<T> *tp)
        //也不能访问类里面非static成员
        {

            while (true)
            {
                //从任务队列里面去拿一个任务
                //执行任务，要先把这个任务队列锁主
                unique_lock<mutex> lock(tp->mtx_);//这个是定义出来就上锁了，所以不能作为成员对象
                tp->cond_.wait(lock,[&](){return !tp->IsEmpty();});
                //该任务队列里面一定有任务了
                T t;
                tp->PopTask(&t);
                
                //任务就拿到了
                t(); //可能有多个线程在处理任务,

                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
        void InitThreadPool()
        {
            vector<thread> vths;
            vths.resize(num_);//初始化一些线程
            for(int i=0;i<num_;i++)
            {
                vths[i]=thread(Rountine,this);//因为类里面的函数都有this指针，
                vths[i].detach();
            }
        }
        void PopTask(T *out)
        {
            *out = task_queue_.front();
            task_queue_.pop();
        }
        
        bool IsEmpty()
        {
            return task_queue_.empty();
        }
       
        
        void PushTask(const T &in)
        {
            //塞任务，就相当于一个生产者，生产者之间要进行互斥访问
            unique_lock<mutex> lock(mtx_);
            task_queue_.push(in);
            cond_.notify_one();
        }

        //万一任务队列里面一个任务都没有的话，那么线程池里面的每一个线程就要处于休眠状态，挂起等待
    };
    template <class T>
    //静态成员变量的初始化必须要在类外面初始化
    ThreadPool<T> *ThreadPool<T>::ins = nullptr; //将threadpool里面的ins进行初始化，返回值是指针，给它初始化为空，说明没有被创建出来

}