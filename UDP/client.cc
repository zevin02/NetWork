#include <iostream>
using namespace std;
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include<cstring>
#include<cstdio>
//要知道server对应的ip和port
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "ip+port" << endl;
        return 0;
    }
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        cerr << "socket error" << endl;
        return 1;
    }
    //这里就不需要再继续绑定了，直接发送数据过去就可以了，不需要显示的bind
    // 1.首先客户端也必须要有ip和port
    // 2，但是客户端不需要显示bind，一旦显示bind，就必须明确，client和哪一个port进行关联，那么有可能会出现冲突
    //有可能被占用，就会导致客户端无法使用，服务器用的是port必须明确，而且不变但client只要有就可以了
    //一般是操作系统自动绑定的，就是client正常发送数据的时候，操作系统就会自动绑定

    //使用服务
    while (1)
    {
        //发送数据
        //数据从哪里来，要给谁发送
        string msg;
        // cin >> msg;
        // getline(cin,msg);
        cout<<"Myshell$ ";
        char line[1024];
        fgets(line,sizeof(line),stdin);//从键盘里面读取

        //因为我们要发送的是命令，所以是一整行
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(atoi(argv[2]));
        server.sin_addr.s_addr = inet_addr(argv[1]);
        sendto(sockfd, line, strlen(line), 0, (struct sockaddr *)&server, sizeof(server));//发送的大小是不包含\0

        //接收
        struct sockaddr_in tmp;
        socklen_t len = sizeof(tmp);
        char buf[1024];
        ssize_t s = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&tmp, &len); // tmp就是一个占位符的概念，里面没有什么用
        //会接收到接收的字节数
        if (s > 0)
        {
            //假如说发送的是hello，5个字节
            //我们接收端把它当作字符串，就要在最后一个位置加上\0
            buf[s] = 0;
            cout << buf << endl;
        }
        else
        {
            cerr<<"recvfrom error"<<endl;
        }
    }

    return 0;
}