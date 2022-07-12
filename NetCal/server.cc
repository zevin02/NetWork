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
    // 1.读取请求,但是这样的操作对于90%的情况是可以满足的,但是对于一些老的服务器就不可以使用了

    //直接发的话缺少了一个序列化和放序列化的过程
    ;
    char buf[1024];

    ssize_t s = read(sockfd, buf, sizeof(buf) - 1);
    if (s < 0)
    {
        cout << "error" << endl;
        close(sockfd);
    }
    else if (s == 0)
    {
        cout << "client quit..." << endl;
        close(sockfd);
    }
    else
    {
        //只要大于0就认为读取成功了
        buf[s] = 0;
        string msg = buf;
        request_t req;
        //进行对字符串的反序列化请求
        ReqReSerialize(msg, req);

        //读取过来要进行一个反序列化的过程

        // if (s == sizeof(req)) //因为传送过来的是一个结构体,所以就是==
        // {
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
        cout<<"response "<<resp.result<<endl;
        //这次我们要先对resp进行序列化
        string send_msg = RespSerialize(resp);
        cout<<send_msg<<endl;
        write(sockfd, send_msg.c_str(), send_msg.size()); //序列化之后再发送回去
        cout << "server finish" << endl;
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