#include"server.hpp"
int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        cout<<"wrong command,please type again"<<endl;
        exit(1);
    }
    TCPSERVER* p=new TCPSERVER(atoi(argv[1]));
    p->InitServer();
    p->StartTcp();
    delete p;
    return 0;
}