#include<iostream>
using namespace std;
#include<sys/types.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<cstring>
#include<cstdlib>
#include<unistd.h>

//发送指定长度的字符串
int writen(int fd,const char* msg,int size)
{
    const char * buf=msg;//buf指向的是msg的首地址
    int count=size;//剩余的长度没有被发送出去的字节数
    while(count>0)
    {
        //不停的进行数据发送
        int len=send(fd,buf,count,0);//send成功返回发送出去的字节数，否则失败返回-1，fd为向哪一个文件描述符里面发送
        //buf是发送的数据
        //count是数据的长度
        if(len==-1)
        {
            //发送失败
            return -1;
        }
        else if(len==0)
        {
            //一个字节都没有发送出去
            continue;//再发送一次
        }
        else
        {
            buf+=len;//buf这个指针往后移动
            count-=len;//count为剩余的字节数，变成0的话就发送完成了
        }
    }
    return size;//发送成功

}


//这个加包头的操作就是这样了，其他客户端该怎么发还是怎么发送
//发送数据
int sendmsg(int cfd,const char* msg,int len)
{
    if(cfd<0||msg==nullptr||len<=0)
    {
        exit(1);
    }
    char * data=(char*)malloc(sizeof(len+4));//先动态申请一些内存，+4是为了存数据头
    //把要发送的数据的长度先转化成网络字节序
    int biglen=htonl(len);
    memcpy(data,&biglen,4);//把biglen的浅4个字节拷贝到data里面
    //把我们需要的数据也拷贝到这一个内存里面去
    memcpy(data+4,msg,len);
    //数据拷贝完之后就要发送数据了
    int ret=writen(cfd,msg,len+4);//+4是因为要加上这个数据的包头
    if(ret==-1)
    {
        close(cfd);//函数调用失败,把文件描述符关掉
    }
    //发送完之后再把内存给释放掉
    free(data);
}



//接收端
//接收指定字节个数

int readn(int fd,char* buf,int size)//buf里面就是我们要把数据读取到的地方
{
    //我们需要往buf这个内存地址里面写数据了，所以不能加const
    //我们需要记录还需要读取多少个字节，以及读取到的位置
    char* pt=buf;
    int count=size;//我们剩余要接收的字节数
    while(count>0)
    {
        int len=recv(fd,pt,count,0);//pt我们需要读取的地址，count就是我们需要读取的字节数，len就是实际读取到的长度
        if(len==-1)
        {
            //读取失败
            return -1;
        }
        else if(len==0)
        {
            //发送端已经断开了连接
            return size-count;//我们就返回收到的字节数
        }
        else
        {
            //正常的读取了
            pt+=len;
            count-=len;
        }
    }
    return size;//成功返回
}


//接收函数
int recvmsg(int fd,char** msg)//这里的msg是一个输出型参数
{
    //我们需要先把数据头给读出来，看它的数据是有多少的数据
    int len=0;
    readn(fd,(char*)&len,4);//我们把数据读取到len里面
    //现在还是网络字节序，我们需要将它转化为主机字节序
    len=ntohl(len);
    cout<<"要接收到的数据块的长度为"<<len<<endl;
    //根据我们读取到的长度len（有效数据的大小）来分配长度
    char* data=(char*)malloc(sizeof(len+1));//+1是‘\0’,字符串结束的标志
    //再去调用这个函数
    int length=readn(fd,data,len);//我们要接收的数据长度是len
    if(length==len)
    {
        cout<<"读取成功"<<endl;

    }
    else
    {
        //接收数据失败了
        cout<<"接收数据失败了"<<endl;
        close(fd);
        free(data);//因为接收失败了，所以这块内存就没有意义了
        return -1;
    }
    data[len]='\0';
    *msg=data;

}


//再子线程里面
char* buf;
int len=recvmmsg(fd,&buf);//buf里面的就是存放的数据
if(len>0)
{
    //成功
    /*
    执行完之后，一系列操作之后，再把数据发送出去
    free(buf)
    */
}

int main()
{
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
    {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in caddr;
    memset(&caddr,'\0',sizeof(caddr));
    caddr.sin_family=AF_INET;
    caddr.sin_port=htons(8081);
    caddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    connect(fd,(struct sockaddr*)&caddr,sizeof(caddr));
    cout<<"客户端连接成功"<<endl;

    return 0;
}