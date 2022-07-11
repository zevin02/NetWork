#include "protocol.hpp"
#include "Sock.hpp"
void *HandlerRequest(void *args)
{
    pthread_detach(pthread_self());
    int sockfd = *(int *)args;
    delete args;

    //业务逻辑，先读先要放序列化，然后计算，判断结果是否正确，正确返回，不正确异常
    while (1)
    {
        char buf[1024];
        read(sockfd, buf, sizeof(buf));
        cout << buf << endl;
    }
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "ip" << endl;
        exit(1);
    }
    uint16_t port = atoi(argv[1]);
    int sockfd = Sock::Socket();
    Sock::Bind(sockfd, port);
    cout<<"1"<<endl;

    Sock::Listen(sockfd);
    while (true)
    {
        int newsockfd = Sock::Accept(sockfd);
        if (newsockfd < 0)
        {
            continue;
        }
        pthread_t tid;
        int *pram = new int(newsockfd);
        pthread_create(&tid, nullptr, HandlerRequest, (void *)pram);
    }

    return 0;
}