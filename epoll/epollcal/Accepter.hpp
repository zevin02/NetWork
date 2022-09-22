#pragma once
#include"sock.hpp"
#include"reactor.hpp"
#include"service.hpp"
#include"Util.hpp"
int Accepter(Event* evp)
{
    //如果，一次有好几个链接呢，accept只读一次，其他的fd没读，以后都不会读取
    //所以我们
    cout<<"有新的链接上来了,就绪的socket是:"<<evp->sockfd<<endl;
    int listensock=evp->sockfd;
    while(true)
    {
       int sock=Sock::Accept(evp->sockfd);
       if(sock<0)
       {
           cout<<"Accept done"<<endl;
           //现在我们就认为它读完了
           break;
       }
       cout<<"accept succsee:"<<sock<<endl;
        //这里获取链接成功了
        //只要封装成Event，添加到epoll里面就行了
        SetNonBlock(sock);//把套接字设置为非阻塞
        Event* otherev=new Event();
        otherev->sockfd=sock;
        otherev->R=evp->R;//它evp里面的R指向哪个唯一的Reactor，新来的R也指向哪个Reactor
        //为什么要让每一个Event指向自己所属的Reactor
        
        otherev->RegisterCallback(Recver,Sender,Errer);//这里就是真正IO的socket,注册这些回调方法
        //如果普通fd触发了，就会调用这些方法，因为这些我们都提前触发了

        evp->R->InsertEvent(otherev,EPOLLIN|EPOLLET);//这样就能直接掉用Reactoer的方法，新链接上的fd也都是用ET模式
    }
}