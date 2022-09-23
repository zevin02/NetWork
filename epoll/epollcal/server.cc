//我们需要给每一个fd，都要有专属自己独立的输入输出缓冲区
//虽然已经对等和拷贝在接口层面已经分离，但是在代码逻辑上依旧是耦合的
// epoll最大的优势在于就绪事件通知机制
#include "reactor.hpp"
#include "sock.hpp"
#include"Accepter.hpp"
#include"Util.hpp"

//Reactor反应堆模式：通过多路转接方案，被动的采用事件派发的方式，反向的调用对应的回调函数
//每个事件加入反应堆之前都已经设置了自己如果触发该使用什么方法

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "port" << endl;
        exit(1);
    }
    int listenfd=Sock::Socket();
    SetNonBlock(listenfd);//设置为非阻塞
    Sock::Bind(listenfd,(uint32_t)atoi(argv[1]));
    Sock::Listen(listenfd);

    //创建一个Reactor对象
    //1.要检测到事件
    //2.派发事件--- Dispatcher(派发+IO)+业务处理（线程池）,半同步半异步的处理，用到的最多就是Reactor模式
    //3.连接
    //4.IO---recver,sender


    Reactor* R=new Reactor();
    R->InitReactor();

    //给Reactor反应堆中加柴火
    //3.1有柴火
    Event* evp=new Event();

    evp->sockfd=listenfd;
    evp->R=R;//每一个柴火都要知道自己对应的Reactor对象
    //Accepter:链接管理器
    evp->RegisterCallback(Accepter,nullptr,nullptr);
    //将准备好的柴火放到反应堆里面
    
    R->InsertEvent(evp,EPOLLIN|EPOLLET);//管理到Reactor里面,未来所有的fd都要是ET模式

    //开始进行事件派发
    int timeout=1000;
    while(true)
    {
        R->Dispatcher(timeout);//服务器正常的惊醒派发

    }

    return 0;
}