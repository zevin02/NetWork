#include "protocol.hpp"
#include "Sock.hpp"
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "ip+port" << endl;
        exit(1);
    }
    uint16_t port = atoi(argv[2]);
    int sockfd = Sock::Socket();
    Sock::Connect(sockfd, argv[1], port);
    while (1)
    {
        string msg;
        getline(cin, msg);
        write(sockfd, msg.c_str(), msg.size());
    }
    return 0;
}