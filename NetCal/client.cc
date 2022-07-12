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
    cout << "Please Enter Data One# ";
    cin >> req.x;
    cout << "Please Enter Data Two# ";
    cin >> req.y;
    cout << "Please Enter Data Opt# ";
    cin >> req.opt;
    string sendwriter=ReqSerialize(req);
    write(sockfd, sendwriter.c_str(), sendwriter.size());

    //这样就序列化成功了
    //读取信息
    char buf[1024];
    ssize_t s = read(sockfd, buf, sizeof(buf) - 1);
    //对resp进行反序列化
    response_t resp;
    if (s > 0)
    {
        buf[s] = 0;
        string msg = buf;
        // cout<<msg<<endl;
        //对响应进行反序列化完成
        RespReSerialize(msg, resp);
        cout << "code[0:success]: " << resp.code;
        cout << "result " << resp.result << endl;
        // if (s == sizeof(resp))
        // {
        //     //读取成功

        // }
    }
    else
    {
        exit(1);
    }
    return 0;
}