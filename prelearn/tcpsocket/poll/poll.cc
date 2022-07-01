#include<iostream>
#include<unistd.h>
#include<poll.h>
#include<cstdlib>
using namespace std;


int main()
{
    struct pollfd pollfd[1];//这里我们就只监听一个文件描述符
    pollfd[0].fd=0;//监听一下输入的文件描述符测试一下
    pollfd[0].events=POLLIN;
    pollfd[0].revents=0;
    //把监听的事件设置为读事件，我们不监听输出的事件
    while(1)
    {
        int ret=poll(pollfd,1,-1);//-1就是阻塞等待，0是非阻塞等待，>0就是要超时的事件
        //这里的ret返回值和select一样，0就是继续要超时等待下去
        if(ret<0)
        {
            cout<<"poll fail"<<endl;
            exit(1);
        }
        for(int i=0;i<1;i++)
        {
            if(pollfd[i].revents==POLLIN)//revent 里面存放的就是真正的执行了什么事件
            {
                char buf[1024]={0};
                read(pollfd[i].fd,buf,sizeof(buf)-1);
                cout<<buf<<endl;
            }
        }
    }
    return 0;
}
