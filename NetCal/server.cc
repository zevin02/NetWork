#include "protocol.hpp"
#include "Sock.hpp"
void *HandlerRequest(void *args)
{
    pthread_detach(pthread_self());
    int sockfd = *(int *)args;
    delete args;

    //业务逻辑，先读先要放序列化，然后计算，判断结果是否正确，正确返回，不正确异常
    //做一个短服务，request -> 分析处理  ->构建response  ->sent（response）--->close(sock)

    // verson1:没有明显的序列化和反序列化的过程
    // 1.读取请求
    request_t req;
    ssize_t s = read(sockfd, &req, sizeof(req));
    if (s == sizeof(req)) //因为传送过来的是一个结构体,所以就是==
    {
        //读取到了一个完整的请求,待定
        // req.x,req.y,req.opt
        // 2.分析请求
        // 3.计算结果
        response_t resp = {0, 0}; //响应,这里的默认响应结果我们都给他设置为0,默认都设置为0
        // 4.构建响应,并进行返回
        switch (req.opt)
        {
        case '+':
            resp.result = req.x + req.y;
            break;
        case '-':
            resp.result = req.x - req.y;

            break;
        case '*':
            resp.result = req.x * req.y;

            break;
        case '/':
            if (req.y == 0)
                resp.code = -1;
            else
                resp.result = req.x / req.y;
            break;
        case '%':
            if (req.y == 0)
                resp.code = -2;
            else
                resp.result = req.x % req.y;
            break;
        default:
            resp.code = -3; //代表我们的请求方法异常
            break;
        }
        //处理完之后就要返回响应
        cout << "request " << req.x << req.opt << req.y << endl;
        write(sockfd, &resp, sizeof(resp));
    }
    else
    {
        close(sockfd);
    }

    // 5.关闭链接
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
    cout << "1" << endl;

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