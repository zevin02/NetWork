#include"server.hpp"

int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        cerr<<"Usage:"<<argv[0]<<"port:"<<argv[1]<<endl;
        exit(-1);
    }
    int port=atoi(argv[1]);
    UDPSERVER* sev=new UDPSERVER(port);
    sev->UDPINIT();
    sev->UDPSTART();
    delete sev;
    return 0;
}