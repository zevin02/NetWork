#include"client.hpp"
int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        exit(-1);
    }
    int port=atoi(argv[1]);
    UDPCLIENT uc(port,argv[2]);
    uc.UDPINIT();
    uc.UDPSTART();

    return 0;
}