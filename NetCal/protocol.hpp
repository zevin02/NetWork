//我们在通信的时候要自己定制协议
//客户端和服务器要进行计算器的功能，我们要有请求有响应
//这个本质上是一个应用层网络服务
#pragma once
#include <iostream>
#include <string>
#include <pthread.h>
#include <jsoncpp/json/json.h>

using namespace std;

//定制协议的过程，目前就是定制结构化数据的过程
//请求格式
//但是向这种,如果面对的是老客户端,一旦有一个字节没有办法发送过来,就出现了错误

//我们需要序列化这个东西

struct request_t
{
    int x;    // 10
    int y;    // 0
    char opt; //我们协议上是支持  //
    request_t() = default;
}; //请求协议

//这里我们写一个响应格式
struct response_t
{
    int code;   //程序运算完毕的计算状态，code=0（success），code=-1（：\0）,code=-2(%0)，先检测code，得到result才有意义
    int result; //计算结果,能否区分是正常的计算结果，还是异常退出结果
};

//这里的话我们实现一个序列化请求的函数
string ReqSerialize(const request_t &req)
{
    Json::Value root;
    root["one"] = req.x;
    root["two"] = req.y;
    root["operator"] = req.opt;
    Json::FastWriter writer;
    string sendwriter = writer.write(root);
    return sendwriter; //返回序列化之后的字符串
}

//这里实现一个反序列化的函数
// string--->request_t
void ReqReSerialize(const string &jsonstring, request_t &req)
{
    Json::Reader reader;
    Json::Value  root;

    reader.parse(jsonstring, root); //解析进行反序列化
    req.x = root["one"].asInt();
    req.y = root["two"].asInt();
    req.opt =(char) root["operator"].asUInt();
}


//序列化响应的函数
string RespSerialize(const response_t &resp)
{
    Json::Value root;
    root["code"]=resp.code;
    root["result"]=resp.result;
    Json::FastWriter writer;
    string sendwriter = writer.write(root);
    return sendwriter; //返回序列化之后的字符串
}


//这里实现一个反序列化响应的函数
// string--->response_t
void RespReSerialize(const string &jsonstring, response_t &resp)
{
    Json::Reader reader;
    Json::Value  root;

    reader.parse(jsonstring, root); //解析进行反序列化
    resp.code = root["code"].asInt();
    resp.result = root["result"].asInt();
}