#include<iostream>
#include<string>
#include<jsoncpp/json/json.h>

using namespace std;


//仅仅是了解一下序列化和反序列化的过程
struct request_t
{
    int x;    // 10
    int y;    // 0
    char opt; //我们协议上是支持  //
    request_t() = default;
}; //请求协议

int main()
{
    request_t req={10,20,'*'};
    //现在我们有了这个结构化的数据,所以我们就需要把它进行序列化
    Json::Value root;//可以承转任意对象,json是一种kv式的序列化方案
    //要序列化的对象先装到一个value对象里面
    root["datax"]=req.x;
    root["datay"]=req.y;
    root["operator"]=req.opt;

    //FastWriter StyledWriter   有这两种类型
    // Json::StyledWriter writer;//我们定义了一个json里面Writer类,writer对象,这个writer式一种分层的
    Json::FastWriter writer;//我们定义了一个json里面Writer类,writer对象,这个writer式一种分层的
    //而FastWriter 就是一种正常的字符串样子
    string json_string=writer.write(root);//这里的返回值是一个string类型的对象
    //现在我们就完成了一个序列化的过程
    cout<<json_string<<endl;


    //接下来就需要反序列化

    string jsontostruct=R"({"datax":10,"datay":20,"operator":42})";//R是把里面的东西当中最原始的东西来看待,避免对里面的""做转义
    Json::Reader reader;//调用里面的读取
    Json::Value rooter;
    //将字符串翻译成结构化的数据
    reader.parse(jsontostruct,rooter);
    request_t reqr;
    reqr.x=rooter["datax"].asInt();//类似于map,我们定义的datax=x,把它当作一个整数来看待
    reqr.y=rooter["datay"].asInt();
    reqr.opt=(char)root["operator"].asUInt();//这样就可以获得他的对应的东西了
    cout<<reqr.opt<<endl;
    cout<<reqr.x<<reqr.opt<<reqr.y<<endl;
    // cout<<jsontostruct<<endl;
    //这样就读取到里面的内容了

    return 0;
}