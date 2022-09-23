#pragma once
#include <iostream>
using namespace std;
#include <string>
#include <sys/epoll.h>
#include <unordered_map>
#include <cstdlib>
#include<unistd.h>
#define SIZE 64
#define NUM 128

//一般处理IO的时候，我们只有3种接口需要处理
//处理读取
//处理写入
//处理异常
class Event;
typedef int (*callback_t)(Event *ev); //定义了函数指针，重命名为callbacK_t
class Reactor;
//需要让epoll管理的基本节点

class Event
{
    //事件，每个fd有自己专属的缓冲区
public:
    int sockfd;     //对应的文件描述符
    string inbuff;  //对应的fd对应的接收缓冲区,从这里解决粘包问题,把这个全部都读到这里面,先读到这里，后面进行协议分析
    string outbuff; //对应的发送发送缓冲区，所有的response都放在这里

    // sock设置回调
    //通过回调的方式来进行解耦
    callback_t recver; //这里我们可以把所有的改成仿函数
    callback_t sender;
    callback_t errer;

    //试着event回指reactor的指针
    Reactor *R; //这个就可以回指Reactor对象,每个event都是指向同一个Reactor

public:
    Event()
    {
        sockfd = -1;
        recver = nullptr; //初始化，处理的时候判断，如果不为空，它的对应的回调机制就被设置了
        sender = nullptr;
        errer = nullptr;
        R = nullptr;
    }

    //设置3个回调,注册回调方法
    void RegisterCallback(callback_t _recver, callback_t _sender, callback_t _errer)
    {
        recver = _recver;
        sender = _sender;
        errer = _errer;
    }
    ~Event()
    {
    }
};

//不需要关心任何sock的类型（listenfd，读fd，写fd）
//我们只关心如何进行使用该类，对Event进行管理

// Reactor:Event=1:n,每一个Event都能指向唯一的Reactor
class Reactor //反应堆模式，只要哪个事件就绪了，就会自动调用回调
{
private:
    int epfd;
    //我们要把fd和事件对应起来，通过这个fd找到它的读缓冲区，写缓冲区
    unordered_map<int, Event *> events; //用hash来建立kv结构,表示epoll类管理的所有的event的集合

public:
    Reactor()
        : epfd(-1)
    {
    }
    ~Reactor()
    {
    }
    // epoll最大的优势在于，就绪事件通知机制---->就绪事件派发逻辑,写一个派发器，dispatcher
    void InitReactor()
    {
        //先创建一个epoll模型,红黑树，就绪队列，回调方法
        epfd = epoll_create(SIZE);
        if (epfd < 0)
        {
            cerr << "epoll_creat error" << endl; // epoll失败了
            exit(1);
        }
        cout << "InitReactor success" << endl;
    }
    bool InsertEvent(Event *evp, uint32_t evs) // events就是我们要关心的事件
    {
        //向reactor里面插入一个事件
        // 1. 将sock插入到epoll中
        struct epoll_event ev;
        ev.events = evs;
        ev.data.fd = evp->sockfd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, evp->sockfd, &ev) < 0)
        {
            //事件插入失败
            cout << "epoll_ctl add failure" << endl;
            return false;
        }
        //添加到epoll模型成功
        // 2. 将ev本身插入到unordered_map中
        events.insert({evp->sockfd, evp}); // fd和Event
    }
    void DeleteEvent(Event *evp)
    {
        int sock = evp->sockfd;
        //从unordered_map中删除它
        auto iter = events.find(sock);

        if (iter != events.end())
        {

            //从epoll中删除它
            epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr); //将它从epoll树里面删除掉
            //找到了

            events.erase(iter); //从map里面删除掉

            // close
            close(sock);

            //删除event的节点
            delete evp;//这里我们把之前new的数据给删除掉，清理资源

        }
    }
    //关于修改最后再看,它不做任何处理，它只是进行事件的派发

    bool EnableRW(int sock, bool enbread, bool enbwrite) //使能读写
    {
        struct epoll_event ev;
        ev.events = EPOLLET | (enbread ? EPOLLIN : 0) | (enbwrite ? EPOLLOUT : 0); //给他配上能够就绪
        ev.data.fd = sock;
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &ev) < 0) //修改失败
        {
            //修改的读写事件
            cout << "epoll_ctl failure" << endl;
            return false;
        }
    }

    bool IsSockOk(int sock)
    {
        auto iter=events.find(sock);
        return iter!=events.end();//就是再map里面做查找的工作
    }
    void Dispatcher(int timeout) //让我们决定超时事件
    {
        struct epoll_event revs[NUM];
        //对于就绪事件的派发器
        int n = epoll_wait(epfd, revs, NUM, timeout);
        for (int i = 0; i < n; i++)
        {
            //这里所有事件都是就绪的
            //超时和失败不关心
            int sock = revs[i].data.fd;       //这就是我们注册的哪个fd就绪
            uint32_t revent = revs[i].events; //本文件描述符就绪的事件

            //代表差错处理，将所有的出错问题，全部转化成IO函数去解决
            if (revent & EPOLLERR)
            {
                //文件描述符出错
                revent |= (EPOLLIN | EPOLLOUT); //如果有出错事件，我们都把它归类成读事件或者写事件就绪
            }
            if (revent & EPOLLHUP)
            {
                //对端连接关闭,epoll是能够识别的
                revent |= (EPOLLIN | EPOLLOUT); //如果有连接断开事件，我们都把它归类成读事件或者写事件就绪
            }
            //读数据就绪
            if (revent & EPOLLIN)
            {
                //这里我们要保证这个sock没有被删除掉,所以我们要保证sock是有效的
                if (IsSockOk(sock)&&events[sock]->recver) //找到sock对应的事件Event
                //有可能再读的时候已经出错了，但是下面写的时候又出现了错误，
                {
                    //这里不为空，说明这个读回调被设置过,提前注册了方法
                    //这里就调用它的方法,执行对应的读取

                    events[sock]->recver(events[sock]); //放到我的inbuff里面,调用读取的回调函数，我们把所有的差错处理都放在IO流里面
                    //这样就能让读取和IO进行解耦
                    //哪个event就绪了，就把哪个event传进来
                }
            }
            if (revent & EPOLLOUT)
            {
                if (IsSockOk(sock)&&events[sock]->sender)
                {
                    // sender不为空，被设置过了
                    events[sock]->sender(events[sock]); //放到我的sendbuff里面
                }
            }
        }
    }
};