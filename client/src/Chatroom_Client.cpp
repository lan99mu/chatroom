#include "Tools.h"
#include <bits/stdc++.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

#define Port 10000
#define MaxLen 1000
#define Ip "47.94.45.86"

string name = "";
void RecvMsg(int sockfd);
void SendMsg(int sockfd);
int main()
{
    int sockfd;
    struct sockaddr_in so_addr;
    cout << "请输入你的昵称：";
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket is error");
        return 0;
    }
    so_addr.sin_family = AF_INET;
    so_addr.sin_port = htons(Port);
    so_addr.sin_addr.s_addr = inet_addr(Ip);
    memset(so_addr.sin_zero, 0, sizeof(so_addr.sin_zero));
    if(connect(sockfd, (struct sockaddr *)&so_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect is error");
        return 0;
    }
    cin >> name;//获取昵称，并发送给服务器端
    string bits = "";
    Tools::string_to_bits(name, bits);
    if(send(sockfd, bits.c_str(), bits.size(), 0) == -1)
    {
        perror("send is error");
        close(sockfd);
        return 0;
    }
    thread t_recvmsg(RecvMsg, sockfd);//给接收消息分配个线程   
    t_recvmsg.detach();
    thread t_sendmsg(SendMsg, sockfd);//给发送消息分配个线程
    t_sendmsg.detach();
    while(true);//让主线程不退出
}
//接受消息
void RecvMsg(int sockfd)
{
    while(true)
    {
        char buffer[MaxLen];
        int num = recv(sockfd, buffer, MaxLen, 0);
        if(num < 0)
        {
            perror("ercv is error");
            return ;
        }
        buffer[num] = '\0';
        string bits = buffer, msg = "";
        Tools::bits_to_string(bits, msg);//将二进制比特流转换成字符流
        for(int i = 0; i < msg.size(); i ++)
            if(msg[i] == '@')
                msg[i] = ' ';
        cout << msg << endl;
    }
}
//发送消息
void SendMsg(int sockfd)
{
    while(true)
    {
        string msg, bits = "";
        cin >> msg;      
        Tools::string_to_bits(msg, bits);//将字符流转换成比特流
        if(send(sockfd, bits.c_str(), bits.size(), 0) == -1)
        {
            perror("send is error");
            close(sockfd);
            return ;
        }
    }
}
