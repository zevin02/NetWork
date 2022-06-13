#include"client.hpp"
int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        cout<<"please type again"<<endl;
        exit(1);
    }
    TcpClient* cl=new TcpClient(argv[1],atoi(argv[2]));
    cl->InitClient();
    cl->StartClient();
    
    return 0;
}