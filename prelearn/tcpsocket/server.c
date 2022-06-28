#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/select.h>
#define SERVER_PORT 8800
int main()
{
    // 1.先socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //这个是后期和服务器建立连接的
    int maxfd = 0;                                //设置最大fd号,最大的文件描述符
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }
    // 2.bind
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt)); //实现了端口的重复利用
    struct sockaddr_in s;
    s.sin_family = AF_INET;
    s.sin_port = htons(SERVER_PORT);       //需要将本地端口字节序转化成网络字节序
    s.sin_addr.s_addr = htonl(INADDR_ANY); //这里面就是我们需要的ip地址,将他转化成为网络字节序

    int d = bind(sockfd, (struct sockaddr *)&s, sizeof(s));
    if (d != 0)
    {
        perror("bind");
        exit(1);
    }
    listen(sockfd, 128); //进行监听

    fd_set rset, tmpset; // rset就是一个读事件，allset就是作为一个rset在select之后的一个备份
    //一开始
    maxfd = sockfd;
    FD_ZERO(&tmpset);        //先清零
    FD_SET(sockfd, &tmpset); //把sockfd放到这个位图里面去,我要监听的就是sockfd
    int nready = 0;          // select里面有多少个文件描述符准备好了
    int confd;
    int maxi=-1;//用来检索的那个数组的下标
    int clentfd[1024];
    for(int i=0;i<1024;i++)
    {
        clentfd[i]=-1;//把里面的值都清理成-1了
    }
    while (1)
    {
        //如果不循环的话，就只能监听一次
        rset = tmpset;                                       //做一个备份
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL); //最后一个参数我们设置为NULL，就是阻塞等待
        //我们只需要把要监听的文件描述符放到数组里面去
        if (nready < 0)
        {
            perror("select"); //调用失败
            exit(1);
        }
        if (FD_ISSET(sockfd, &rset)) //监听里面一定有读事件，同时sockfd也在rset里面，我们检测一下sockfd是否还在位图里面
        {
            //这里的rset是传出
            //客户端一定是发出了连接请求,这里肯定是用server来进行连接

            //如果nready=2，那么就是有一个文件描述符连接上来，有一个文件描述符有数据要发送
            // sockfd里面有事件说明的是有人要连接上来

            struct sockaddr_in clie;
            socklen_t addrlen = sizeof(clie);
            confd = accept(sockfd, (struct sockaddr *)&clie, &addrlen);
            //把这个confd也添加到tmpset里面去
            int i;
            for(i=0;i<1024;i++)
            {
                if(clentfd[i]<0)
                {
                    clentfd[i]=confd;
                    break;
                }
            }
            if(i==1024)
            {
                exit(1);//越界了
            }

            FD_SET(confd, &tmpset); //把这个confd也添加到里面去tmpset里面
            if (maxfd < confd)
            {
                maxfd = confd;
            }
            if(i>maxi)
            {
                maxi=i;//检索最大的上限
            }
            if (nready==1) //如果不是1，就继续往下面判断那个2代表的是哪一个文件描述符
            {
                // nready=1的时候即》nready里面只有sockfd，没有其他的东西
                continue; //就直接返回就行了,后面操作就不需要我们去做了
            }
        }
        //如果nready
        //假如说我们一共监听5个返回的3，假如说sockfd是有人要连接上的事件，那么还有2个要检测是哪两个有事件，就要从sockfd+1开始】
        for (int i = 0; i <= maxi; i++)
        {
            //因为我们已近处理掉sockfd了,这里的i就是我们要判断的文件描述符
            if (FD_ISSET(i, &rset))
            {
                //如果在的话，说明i对应的套接字有事件发生，就是读事件
                char str[100];
                int len = read(i, str, sizeof(str));

                if (len<0)
                {
                    perror("read");
                    exit(1);
                }
                else if(len==0)
                {
                    close(i);
                    //对端关闭
                    FD_CLR(i,&tmpset);//把这个i就不要再监听了，从集合里面移出去, 从总的监听集合里面移除出去,因为我们循环的时候，会对rset进行赋值，再进行监听
                }
                else
                {
                    str[len]=0;
                    printf("server send message %s",str);
                }
            }
        }
    }

    close(sockfd);

    return 0;
}