#pragma once
#include<iostream>
#include<cstdlib>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<cstring>
#include<arpa/inet.h>

int writen(int _sockfd,const char *msg, int size)
{
    const char *buf = msg; //指向它的地址，发送出去
    int count = size;      //剩余需要发送的字节数
    while (count > 0)
    {
        int len = send(_sockfd, buf, count, 0);
        if (len == -1)
        {
            //发送失败
            return -1;
        }
        else if (len == 0)
        {
            continue; //没发送出去，再发送一次
        }
        else
        {
            //发送成功
            buf += len;
            count -= len;
        }
    }
    return size; //发送成功，发送完成
}

void sendmsg(int _sockfd,const char *msg, int len)
{
    //先申请包头
    char *data = (char *)malloc(sizeof(char) * (len + 4)); //多加的4是为了数据的长度
    int biglen = htonl(len);
    memcpy(data, &biglen, 4);        //拷贝4个字节过去
    memcpy(data + 4, msg, len);      //拷贝len个长度过去
    int ret = writen(_sockfd,data, len + 4); //真正的传输数据
    if (ret == -1)
    {
        //发送失败
        free(data); //把data的内存销毁掉
        close(_sockfd);
    }
    else
        free(data);
}

int readn(int _sockfd,char *buf, int size)
{
    char *pt = buf;
    int count = size;
    while (count > 0)
    {
        int len = recv(_sockfd, pt, count, 0);
        if (len == -1)
        {
            //读取失败
            return -1;
        }
        else if (len == 0)
        {
            return size - count;
        }
        else
        {
            pt += len;
            count -= len;
        }
    }
    return size;
}
int RecvMsg(int _sockfd,char **buf)
{
    //解包

    int len = 0;
    readn(_sockfd,(char *)&len, 4);
    len = htonl(len);
    char *msg = (char *)malloc(sizeof(char) * (len + 1));
    int size = readn(_sockfd,msg, len);
    if (size != len)
    {
        close(_sockfd);
        free(msg);
        return -1;
    }
    msg[size] = '\0';
    *buf = msg;
    return size;
}