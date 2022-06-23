#include <iostream>
using namespace std;
#include <fcntl.h>
#include <unistd.h>
#include<errno.h>

void SetNonBlock(int fd)
{
    //获取之前文件的状态
    int fl=fcntl(fd,F_GETFD);
    if(fl<0)
    perror("fcntl");
    //把文件描述符设置为非阻塞,设计标记 
    fcntl(fd,F_SETFL,fl|O_NONBLOCK);
}
int main()
{
    //观察标准输入阻塞和非阻塞状态读取数据
    char ch;
    SetNonBlock(0);//给0设置为非阻塞
    while (1)
    {
        sleep(1);
        ssize_t s = read(0, &ch, 1);
        if (s > 0)
        {
            printf("%c\n", ch); //读取成功
        }
        else if(s<0&&(errno==EAGAIN||errno==EWOULDBLOCK))
        {
            //非阻塞读取，底层的数据没有就位 
            cout<<"continue"<<endl;
        }
        else if(errno==EINTR&&s<0)//读取被信号中断了
        {
            continue;
        }
        else 
        {
            // cout << ch << endl;
            cout<<s<<endl;
        }
        cout << "............." << endl;
    }

    return 0;
}

//当我们不输入的时候，就会卡住等待我们进行输入
//设置为非阻塞，当缓冲区里面没有数据的时候，read直接返回失败，ssize_t 是一个有符号整数，-1代表底层数据没有就绪，
//读取数据不算错误，而是一种通知，并且会设置errno为EAGAIN,(try again)表示底层数据没有准备好，下次再来的话，EWOULDBLOCK也同样的效果
//如果错误码是EINT表示阻塞等待时候被信号给阻塞掉了


