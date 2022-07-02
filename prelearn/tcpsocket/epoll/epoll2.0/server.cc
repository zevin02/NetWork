#include"server.hpp"

int main()
{
    Server sv(8080);
    sv.InitServer();
    sv.StartServer();
    return 0;
}
/*
    select  函数解析
    select会随着文件描述符数量的增加而导致效率下降
    select执行成功之后会把没有就绪的文件描述符给去掉
    


*/
