#pragma once
//工具类
#include<iostream>
#include<fcntl.h>
#include<unistd.h>
#include<string>
#include<vector>
using namespace std;

void SetNonBlock(int sock)//设置非阻塞
{
    int fl=fcntl(sock,F_GETFL);
    if(fl<0)
    {
        std::cout<<"fcntl fail"<<std::endl;
    }
    fcntl(sock,F_SETFL,fl|O_NONBLOCK);//设置非阻塞

}

void SplitSegment(string& inbuffer,vector<string>* tokens,string sep )
{
    //单纯的&是输入输出型参数
    //* 是输出型参数
    int prepos=0;
    while(true)
    {
        auto pos=inbuffer.find(sep,prepos);
        if(pos==string::npos)
        {
            //没有找到X，说明读取的报文不完整
            break;
        }
        //读取的是完整的报文
        string sub=inbuffer.substr(prepos,pos);//从0开始读pos个字节，来截取
        prepos=pos+1;
        tokens->push_back(sub);//这里面存放的就是一个一个的报文1+2,3*4这样有效的报文

        //这里就读取了报文，把刚才读走的数据给移除
        inbuffer.erase(0,pos+sep.size());//移除前面的有效数据，以及用来分割的东西
    }

}

bool DeSerialize(string& seg,string *out1,string* out2)
{
    //1+2
    auto pos=seg.find("+");
    if(pos==string::npos)return false;
    *out1=seg.substr(0,pos);
    *out2=seg.substr(pos+1);
    return true;
}