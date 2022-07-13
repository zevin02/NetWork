#pragma once
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include<sys/epoll.h>
using namespace std;

namespace ns_epoll
{
    class Epoll;
    class Event;
    typedef void (*rollback_t)(Event &); //把这个函数指针重命名成rollback_t

    class Event
    {
    public:
        int _fd;
        Epoll *_r;
        string _inbuffer;
        string _outbuffer;
        rollback_t _readRollback;
        rollback_t _writeRollback;
        rollback_t _errorRollback;
        Event()
            : _fd(-1), _r(nullptr), _inbuffer(nullptr), _readRollback(nullptr), _writeRollback(nullptr), _errorRollback(nullptr)
        {
        }
    };

    class Epoll
    {
    private:
        int _epollfd;
        unordered_map<int, Event> _fdtoevent;//映射

    public:
        void InitEpoll()
        {
            _epollfd=epoll_create(1);//先创建一个句柄
            if(_epollfd<0)
            {
                perror("epoll_create");
                exit(2);
            }
        }
        void AddEvent(const Event& event,uint32_t events)
        {
            int fd=event._fd;
            cout<<fd<<"is add event"<<endl;
            struct epoll_event eve;
            //设置事件
            _fdtoevent[fd]=event;//用fd映射event
            eve.data.fd=fd;
            //我们默认关心读事件，写事件选择关心
            eve.events=events|EPOLLET;
            epoll_ctl(_epollfd,EPOLL_CTL_ADD,fd,&eve);//把这个fd添加进去


        }
        //处理Epoll模型里面的就绪事件
        void Dispatch(int timeout)
        {
            #define NUM 100
            struct epoll_event events[NUM];//我们这里设置这些事件，之后要把它放到epoll反应堆里面，去检测反应
            int num=epoll_wait(_epollfd,events,NUM,timeout);//这里我们设置成阻塞等待
            if(num<0)
            {
                perror("epoll_wait");
                exit(3);
            }
            for(int i=0;i<num;i++)
            {
                //因为Epoll模型都是从AddEvent来的，所以里面的方法都是已近注册好的了
                int fd=events[i].data.fd;
                Event& ev=_fdtoevent[fd];//因为这里我们提前都已近映射好了，所以我们就可以直接获得这个事件，
                //错误都是读写事件来检测
                if(events[i].events&EPOLLERR) 
                events[i].events!=
            }
        }
    };
};