#pragma once
#include <iostream>
#include "sock.hpp"
#include "IO.hpp"
using namespace std;

#define NUM 1024
#define DEL_NUM -1
class Server
{
private:
  int _sockfd;
  int _port;
  int rfd_array[NUM]; //这个数组里面存放着我们需要的那些要响应的文件描述符
public:
  Server(int port = 8088)
      : _port(port), _sockfd(-1)
  {
  }
  void Init()
  {
    //我们先把所有的文件描述符都设置成-1
    for (int i = 1; i < NUM; i++)
    {
      rfd_array[i] = DEL_NUM;
    }
    _sockfd = Sock::Socket();                // 通过静态的函数类，获得里面的文件描述符
    Sock::SetSockOpt(_sockfd);               //端口复用
    Sock::Bind(_sockfd, _port, "127.0.0.1"); //
    Sock::Listen(_sockfd);
    //初始化rfd_array[0]为lsock，用来接收连接
    rfd_array[0] = _sockfd;
  }
  void Start()
  {
    int maxfd = _sockfd; // select的最大值
    fd_set rfds,allset;
    FD_ZERO(&allset);
    FD_SET(_sockfd,&allset);
    while (1)
    {
      rfds=allset;
      //每次time_out都要设置一次rfds，成本很高
      //是否返回值为0的时候，不需要走这里
      for (int i = 0; i < NUM; i++)
      {
        if (rfd_array[i] != -1)
        {
          //如果它下标对应的值不为-1，所以就说明，它是一个由效值
          FD_SET(rfd_array[i], &allset); //把这个有效值放进去，因为它是循环判断的
          cout << rfd_array[i] << endl;
          // 更新maxfd
          if (rfd_array[i] > maxfd)
          {
            maxfd = rfd_array[i];
          }
        }
      }
      //每隔5秒select不成功返回一次，
      struct timeval timeout = {5, 0};
      // switch (select(maxfd + 1, &rfds, nullptr, nullptr, nullptr)) // select调用之后rfds里面返回的是真正运用的
      // {
      // case -1:
      //   perror("select");
      //   break;
      // case 0:
      //   cout << "time out" << endl;
      //   // time out 返回，没有事情发生
      //   break;
      // default:
      //   cout << "event ready" << endl;
      //   Hand(&rfds,maxfd,&allset); //这里面就是read的位图
      //   break;
      // }
      int retval=select(maxfd + 1, &rfds, nullptr, nullptr, nullptr);
      if(retval<0)
      {
        perror("select");
        exit(1);
      }
      else if(retval==0)
      {
        cout<<"nothing happened"<<endl;
      }
      else
      {
        cout<<"event ready"<<endl;
        Hand(&rfds,maxfd,&allset);
      }
    }
  }
  void Add2Rsock(int fd)
  {
    int i = 0;
    for (i = 0; i < NUM; i++)
    {
      if (rfd_array[i] == -1)
      {
        break;
      }
    }
    if (i >= NUM)
    {
      cout << "link num reach the total num" << endl;
    }
    else
    {
      cout << "new index put in " << endl;
      rfd_array[i] = fd; //添加进去
    }
  }
  void Hand(fd_set *rfds,int maxfd,fd_set*allset)
  {
    for (int i = 0; i <= maxfd; i++)
    {
      if (rfd_array[i] != -1)
      {
        // rfds_array[i]当中判断关注那些读文件描述符
        if (FD_ISSET(rfd_array[i], rfds)) //为真就是我们要的文件描述符在这里面
        {
          //护理事情
          if (rfd_array[i] == _sockfd)
          {
            //做新的连接
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int fd = accept(_sockfd, (struct sockaddr *)&peer, &len);
            Add2Rsock(fd);
          }
          else
          {
            //数据接收
            char *buf;
            int s = RecvMsg(rfd_array[i], &buf);
            cout << "read msg" << endl;
            cout<<"s="<<s<<endl;
            if (s <= 0)
            {

              //对端关闭
              cout << "error data" << endl;
              rfd_array[i] = -1; //把它关闭掉
              close(rfd_array[i]);
              FD_CLR(rfd_array[i],allset);

            }
            else
            {

              buf[s] = 0;
              cout << "server recv" << buf << endl;
              string ret = buf;
              ret += " server done";
              sendmsg(rfd_array[i], ret.c_str(), ret.size());
            }
          }
        }
      }
    }
  }
};
