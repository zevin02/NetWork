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
    request_t req;
    memset(&req,0,sizeof(req));
    cout << "Please Enter Data One# ";
    cin >> req.x;
    cout << "Please Enter Data Two# ";
    cin >> req.y;
    cout << "Please Enter Data Opt# ";
    cin >> req.opt;
    write(sockfd, &req, sizeof(req));
    response_t resp;
    ssize_t s = read(sockfd, &resp, sizeof(resp));
    if (s == sizeof(resp))
    {
        //读取成功
        cout << "code[0:success]: " << resp.code;
        cout << "result " << resp.result << endl;
    }
    else
    {
        exit(1);
    }
    return 0;
}