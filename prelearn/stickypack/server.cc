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

int readn(int fd,)

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