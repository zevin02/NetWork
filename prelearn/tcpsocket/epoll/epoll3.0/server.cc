#include"sock.hpp"
#include"epoll.hpp"
using namespace ns_epoll;
int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        cout<<"port"<<endl;
        exit(1);
    }
    Epoll* p=new Epoll;
    p->InitEpoll();//初始化
    
    //启动服务
    int sockfd=Sock::Socket();
    Sock::SetSockOpt(sockfd);
    uint16_t port=atoi(argv[1]);
    Sock::Bind(sockfd,port,"127.0.0.1");
    Sock::Listen(sockfd);

    //设置监听套接字进Epoll模型
    Event eve;//定义一个事件类，里面都已经初始化好了
    //这里因为外面使用的是public：所以我们可以访问到里面的对象
    eve._fd=sockfd;
    eve._r=p;
    eve._writeRollback=nullptr;
    eve._errorRollback=nullptr;
    eve._readRollback=ListenRollBack;
    p->AddEvent(eve,EPOLLIN|EPOLLET);//这里设置成ET模式，这里我们把监听描述符添加了进去
    int timeout=-1;
    while(true)
    {
        p->Dispatch(timeout);
    }


    return 0;
}