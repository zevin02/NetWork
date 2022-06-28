#pragma once
#include <iostream>
#include "sock.hpp"
#include "IO.hpp"
using namespace std;

#define NUM 1024
#define DEL_NUM -1
// class Server
// {
// private:
//     int _sockfd;
//     int _port;
//     int rfd_array[NUM]; //这个数组里面存放着我们需要的那些要响应的文件描述符
// public:
//     Server(int port = 8088)
//         : _port(port), _sockfd(-1)
//     {
//     }
//     void Init()
//     {
//         //我们先把所有的文件描述符都设置成-1
//         for (int i = 1; i < NUM; i++)
//         {
//             rfd_array[i] = DEL_NUM;
//         }
//         _sockfd = Sock::Socket();                // 通过静态的函数类，获得里面的文件描述符
//         Sock::SetSockOpt(_sockfd);               //端口复用
//         Sock::Bind(_sockfd, _port, "127.0.0.1"); //
//         Sock::Listen(_sockfd);
//         //初始化rfd_array[0]为lsock，用来接收连接
//         rfd_array[0] = _sockfd;
//     }
//     void Start()
//     {
//         while (1)
//         {
//             //每次time_out都要设置一次rfds，成本很高
//             //是否返回值为0的时候，不需要走这里
//             int maxfd = _sockfd; // select的最大值
//             fd_set rfds;
//             FD_ZERO(&rfds);
//             cout << "FDS_ARRAY" << endl;
//             for (int i = 0; i < NUM; i++)
//             {
//                 if (rfd_array[i] != -1)
//                 {
//                     //如果它下标对应的值不为-1，所以就说明，它是一个由效值
//                     FD_SET(rfd_array[i], &rfds); //把这个有效值放进去，因为它是循环判断的
//                     cout << rfd_array[i] << endl;
//                     // 更新maxfd
//                     if (rfd_array[i] > maxfd)
//                     {
//                         maxfd = rfd_array[i];
//                     }
//                 }
//             }
//             //每隔5秒select不成功返回一次，
//             struct timeval timeout = {5, 0};
//             switch (select(maxfd + 1, &rfds, nullptr, nullptr,nullptr))
//             {
//             case -1:
//                 perror("select");
//                 break;
//             case 0:
//                 cout << "time out" << endl;
//                 // time out 返回，没有事情发生
//                 break;
//             default:
//                 cout<<"event ready"<<endl;
//                 Hand(&rfds); //这里面就是read的位图
//                 break;
//             }
//         }
//     }
//     void Add2Rsock(int fd)
//     {
//         int i = 0;
//         for (i = 0; i < NUM; i++)
//         {
//             if (rfd_array[i] == -1)
//             {
//                 break;
//             }
//         }
//         if (i >= NUM)
//         {
//             cout << "link num reach the total num" << endl;
//         }
//         else
//         {
//             cout<<"new index put in "<<endl;
//             rfd_array[i] = fd; //添加进去
//         }
//     }
//     void Hand(fd_set *rfds)
//     {
//         for (int i = 0; i < NUM; i++)
//         {
//             if (rfd_array[i] != -1)
//             {
//                 // rfds_array[i]当中判断关注那些读文件描述符
//                 if (FD_ISSET(rfd_array[i], rfds))
//                 {
//                     //护理事情
//                     if (rfd_array[i] == _sockfd)
//                     {
//                         //做新的连接
//                         struct sockaddr_in peer;
//                         socklen_t len = sizeof(peer);
//                         int fd = accept(_sockfd, (struct sockaddr *)&peer, &len);
//                         Add2Rsock(fd);
//                     }
//                     else
//                     {
//                         //数据接收
//                         char *buf;
//                         int s = RecvMsg(_sockfd, &buf);
//                         cout<<"read msg"<<endl;
//                         if (s <= 0)
//                         {

//                             //对端关闭
//                             close(rfd_array[i]);
//                             rfd_array[i] = -1; //把它关闭掉
//                         }
//                         else
//                         {

//                             buf[s] = 0;
//                             cout << "server recv" << buf << endl;
//                             string ret=buf;
//                             ret+=" server done";
//                             sendmsg(_sockfd,ret.c_str(),ret.size());
//                         }
//                     }
//                 }
//             }
//         }
//     }
// };






#define NUM (sizeof(fd_set)*8) 
//#define NUM  1000 
#define DEL_NUM -1

class Server
{
  private:
    int lsock;
    int port;
    int rfds_array[NUM];
  public:
    Server(int p = 8000):lsock(-1),port(p)
    {}

    void Init()
    {
      for(int i = 1; i < NUM ;++i)
      {
        //将默认值设置为DEL_NUM
        rfds_array[i] = DEL_NUM;
      }

      lsock = Sock::Socket();
      Sock::Setsockopt(lsock);
      Sock::Bind(lsock,port);
      Sock::Listen(lsock);
      //初始化rfds_array[0]为lsock，负责接受链接
      rfds_array[0] = lsock;
    }

    void DelFd(int index)
    {
     rfds_array[index] = DEL_NUM; 
    }

    void Add2RSock(int fd)
    {
      int i =0; 
      
      for(; i < NUM; ++i)
      {
        if(rfds_array[i] == DEL_NUM)
        {
          break;
        }
      }
      if(i >= NUM)
      {
        //链接数到达上线，无法处理链接
        printf("link num reach the total num\n");
      }
      else 
      {
        cout << "new link index: "<<i <<endl;
        sleep(1);
        rfds_array[i] = fd;
      }

    }
    void HandlerEvents(fd_set* rfds)
    {
      for(int i = 0; i < NUM;++i)
      {//这里实际上只会对有关心的文件描述符做FD_ISSET检测。
        if(rfds_array[i] != DEL_NUM)
        {
          //从rfds_array当中判断关注哪些读文件描述符
          if(FD_ISSET(rfds_array[i],rfds))
          {
            //处理事件
            if(rfds_array[i] == lsock)
            {
              //新的链接就绪
              printf("new link ....\n");
              //新的链接上来需要加入rfds_array当中，
              //因为连接上服务器不一定马上发送数据
              struct sockaddr_in peer;
              socklen_t len = sizeof(peer);
              int fd = accept(lsock,(struct sockaddr*)&peer,&len);
              Add2RSock(fd);
            }
            else
            {
              //data readly
              //bug,读取数据应结合业务
              char buf[10240];
              ssize_t s =recv(rfds_array[i],buf,sizeof(buf)-1,0);
              if(s > 0)
              {
                //这里认为发送成功
                buf[s] =0 ;
                cout << "Server recv# "<<buf<<endl;
              }
              else if(s == 0)
              {
                //写端关闭，读端关闭
                close(rfds_array[i]);
                //从rfds_array当中拿出，无需在进行读等待
                DelFd(i);
              }
              else 
              {
                //读取发生错误，这里处理方式关闭文件描述符
                close(rfds_array[i]);
                //从rfds_array当中拿出，无需在进行读等待
                DelFd(i);
              }
            }
          }
        }
      }
    }

    void Start()
    {
      while(1)
      {
        //每次time_out都需要重新设置rfds，成本高。
        //是否返回值为0的时候不用走这里
        int maxfd = lsock;
        fd_set rfds;
        FD_ZERO(&rfds);
        printf("FDS_ARRAY#");
        fflush(stdout);
        //设置进fd_set哪些文件描述符需要读
        for(int i = 0; i < NUM;++i)
        {
          if(rfds_array[i] != DEL_NUM)
          {
            //rfds_array的下标无意义，值代表文件描述符
            FD_SET(rfds_array[i],&rfds);
            printf("%d ",rfds_array[i]);
            //跟新maxfd
            if(rfds_array[i] > maxfd)
              maxfd = rfds_array[i];
          }
        }
        cout << endl;
        //每隔5sselect不成功返回一次,注意在循环内部每次都需要重新定义
        struct timeval timeout = {5,0};
        switch(select(maxfd+1,&rfds,nullptr,nullptr,&timeout))
        {
          case -1:
            //失败
            perror("select error");
            break;
          case 0:
            //time_out返回，但无事件就绪
            printf("time_out ......\n");
            break;
          default:
            //有事件就绪
            printf("event readly\n");
            HandlerEvents(&rfds);
            break;
        }

      }
    }
};
