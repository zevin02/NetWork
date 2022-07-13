#include"Sock.hpp"
#include<pthread.h>
#include<cstdio>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/sendfile.h>
#define NUM 1024*10
void* handler(void* args)
{
    int sockfd=*(int*)args;
    delete args;
    pthread_detach(pthread_self());
    //服务器就从sockfd里面读取
    //我们读取http请求
    char buf[NUM];
    memset(buf,0,sizeof(buf));
    //http当中面向字节流读取的就是recv,专门用来网络读取的
    ssize_t s=recv(sockfd,buf,sizeof(buf),0);//
    if(s>0)
    {
        
        buf[s]=0;
        //原封不动的把http打印出来，这个就是http的请求格式! 看看请求的样子
        cout<<buf;
        //读到了报头
        //我们要返回响应
        #if 0
        string response="http/1.0 200 OK\n";//响应行，版本http/1.0 状态码200 状态码解释 OK
        response+="Content-Type: text/plain\n";//text/plain代表正文是普通文本，这个说明了后续的一些正文都是一些普通的文本文件
        response+="\n";//这个是报头结束的标志，空行，区分报头有效载荷
        response+="what are you doing \n";//这个就是正文的内容

        //send也是针对TCP设计的接口
        send(sockfd,response.c_str(),response.size(),0);//因为我们要写的是http响应,所以这里发送的完整的响应发送回去
        #endif

        int fd=open("./index1.html",O_RDONLY);//这里我们打开一个我们写好的一个文件，以只读的方式打开，之后就是把这个内容发送给客户端
        struct stat st;//获得文件的所有状态
        stat("./index1.html",&st);//这里面就有了这个文件的所有信息了
        string response="HTTP/1.1 200 OK\n";//这里填充响应行
        //响应报头
        response+=("Content-Length: "+to_string(st.st_size)+"\n");//正文大小
        response+="Content-Type: text/html\n";//正文类型
        response+="\n";//空行
        cout<<response<<endl;
        send(sockfd,response.c_str(),response.size(),0);//先把报头发送给客户端
        sendfile(sockfd,fd,nullptr,st.st_size);//把正文发送了过去

    }

    close(sockfd);
    return nullptr;
}

int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        cout<<"port"<<endl;
        exit(1);
    }
    uint16_t port=atoi(argv[1]);
    int sockfd=Sock::Socket();
    Sock::Setoptsocket(sockfd);
    Sock::Bind(sockfd,port);
    Sock::Listen(sockfd);
    while(1)
    {
        int newsock=Sock::Accept(sockfd);
        if(newsock<0)
        {
            continue;
        }
        pthread_t tid;
        int* parm=new int(newsock);
        pthread_create(&tid,nullptr,handler,(void*)parm);
    }
    return 0;
}