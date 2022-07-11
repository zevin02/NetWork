//我们在通信的时候要自己定制协议
//客户端和服务器要进行计算器的功能，我们要有请求有响应
#pragma once
#include<iostream>
#include<string>
#include<pthread.h>

using namespace std;



//定制协议的过程，目前就是定制结构化数据的过程
//请求格式
struct request_t 
{
    int x;                      //10
    int y;                      //0
    char opt;//我们协议上是支持  //
};//请求协议


//这里我们写一个响应格式
struct response_t
{
    int code;//程序运算完毕的计算状态，code=0（success），code=-1（：\0），先检测code，得到result才有意义
    int result;//计算结果,能否区分是正常的计算结果，还是异常退出结果
};