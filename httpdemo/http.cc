#include "Sock.hpp"
#include <pthread.h>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <fstream>

#define NUM 1024 * 10

//我们创建了一个目录，就相当于首页wwwroot(http的web根目录,我们在里面创建了一个index.html就相当于首页,把首页信息进行返回)
// WWWROOT就叫做web根目录，wwwroot目录下放置的内容，就叫做资源,这个可以被拷贝到linux目录下的任何一个地方,同时我们里面资源更新，服务器也会更新这个资源

#define wwwroot "./wwwroot/"
#define HOME_PAGE "index.html-a"

void *handler(void *args)
{
    int sockfd = *(int *)args;
    delete args;
    pthread_detach(pthread_self());
    //服务器就从sockfd里面读取
    //我们读取http请求
    char buf[NUM];
    memset(buf, 0, sizeof(buf));
    // http当中面向字节流读取的就是recv,专门用来网络读取的
    //这种读法是不正确的，只不过现在没有暴露出来

    /*
    * 1.http它的请求可能不是一个一个发送的，可能是一次发送多个请求的
    * 2.因为TCP是字节流的，我们上面缓冲区定义的10240，假如说发送过来的请求大小是1024，我们就可以读完，那么如果我们定义的缓冲区是1025，那么就会第一个请求读完之后，第二个请求还会读取1个字节

    所以
    1.保证每次读取都是一个完整的http request
    2.保证每次读取都不要将下一个http request的一部分读到（残缺的报文）


    */
    ssize_t s = recv(sockfd, buf, sizeof(buf), 0); //
    if (s > 0)
    {

        buf[s] = 0;
        //原封不动的把http打印出来，这个就是http的请求格式! 看看请求的样子
        cout << buf;
        /*
            HTTP协议处理，本质就是文本分析
            所谓的文本分析：
            1.http协议本身的字段
            2.第一行拿出来，请求方法，
            里面 kv值，
            3.提取参数，get，post就是前后端交互的重要方式

    */
//读到了报头
//我们要返回响应

        // string response ="http/1.0 301 Permanently move\n";//301永久性重定向
        string response ="http/1.0 302 Found\n";//302临时性重定向
        response+="Location: https://www.qq.com/\n";//这个就是用来搭配3xx来使用的，告诉我们要跳转到哪里去,访问我们的网址之后，就会跳转到qq里面去
        response+="\n";
        send(sockfd,response.c_str(),response.size(),0);//先把报头发送给客户端


#if 0
        string response="http/1.0 200 OK\n";//响应行，版本http/1.0 状态码200 状态码解释 OK
        response+="Content-Type: text/plain\n";//text/plain代表正文是普通文本，这个说明了后续的一些正文都是一些普通的文本文件
        response+="\n";//这个是报头结束的标志，空行，区分报头有效载荷
        response+="what are you doing \n";//这个就是正文的内容

        //send也是针对TCP设计的接口
        send(sockfd,response.c_str(),response.size(),0);//因为我们要写的是http响应,所以这里发送的完整的响应发送回去
#endif
#if 0
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
#endif
#if 0
        string html_file = wwwroot; //首页
        html_file += HOME_PAGE;     //这个就是默认文件
                                    //但是我们需要分析请求里面的目录信息
                                    //返回的时候不仅仅是返回正文网页信息，而是还要包括http的请求

        std::ifstream in(html_file); //打开一个文件
        if (!in.is_open())           //判断打开是否成功
        {
            string response = "http/1.0 404 NOT FOUND\n";
            response += "Content-Type: text/html; charset=utf8\n"; //这里同时设置一下文字的类型
            response+="\n";
            response+="你访问的资源不存在";
            send(sockfd, response.c_str(), response.size(), 0); //先把报头发送给客户端
            cerr << "open html error!" << endl;
        }
        else
        {
            string response = "http/1.0 200 OK\n";
            response += "Content-Type: text/html; charset=utf8\n"; //这里同时设置一下文字的类型
            struct stat st;
            stat(html_file.c_str(), &st);
            off_t size = st.st_size;
            string content;
            string line;
            while (getline(in, line)) // getline第一个是从哪里读，第二个就是读到哪里，
            {
                content += line; //这里就是把网页的信息，按行进行读取，但是不会读取到回车
                content += "\n";
            }

            // cout<<content<<endl;
            // in.read()//读到哪个区域，要读多少字节
            response += content;
            in.close();
            send(sockfd, response.c_str(), response.size(), 0); //先把报头发送给客户端
        }

#endif
    }

    close(sockfd);
    //我们这里的http用的就是短链接，一个请求一个响应，这里用完就把它close掉
    return nullptr;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "port" << endl;
        exit(1);
    }
    uint16_t port = atoi(argv[1]);
    int sockfd = Sock::Socket();
    Sock::Setoptsocket(sockfd);
    Sock::Bind(sockfd, port);
    Sock::Listen(sockfd);
    while (1)
    {
        int newsock = Sock::Accept(sockfd);
        if (newsock < 0)
        {
            continue;
        }
        pthread_t tid;
        int *parm = new int(newsock);
        pthread_create(&tid, nullptr, handler, (void *)parm);
    }
    return 0;
}