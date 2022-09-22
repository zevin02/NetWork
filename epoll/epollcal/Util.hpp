#pragma once
//工具类
#include<iostream>
#include<fcntl.h>
#include<unistd.h>


void SetNonBlock(int sock)//设置非阻塞
{
    int fl=fcntl(sock,F_GETFL);
    if(fl<0)
    {
        std::cout<<"fcntl fail"<<std::endl;
    }
    fcntl(sock,F_SETFL,fl|O_NONBLOCK);//设置非阻塞

}