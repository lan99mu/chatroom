#include "Tools.h"
#include "Client_info.h"
#include <bits/stdc++.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>

#define Max_Conn_num 100
#define MyPort 10000
#define MaxLen 1000
#define MyIp "172.17.179.94"
using namespace std;

set <Client_info> client_set;//存客户端连接信息
vector <Client_info> delete_set;//存要被删除的迭代器
int epollfd = epoll_create(Max_Conn_num);//创建epoll
struct epoll_event events[Max_Conn_num];
int socknum = 0;
mutex g_mutex;//互斥锁

int socket_init_bind();
void Conn(int);
void Get_Data();
void allSend(const char *, int&);
void addEvent(const int&, int);
void deleteEvent(const int&, int);
void allnameSend(int&);


int main()
{
    daemon(0, 0);
    int sockfd = socket_init_bind();
    if(listen(sockfd, Max_Conn_num) == -1)
    {
        perror("listen is error");
        return 0;
    }
    thread t_conn(Conn, sockfd);
    t_conn.detach();
    thread t_getdata(Get_Data);
    t_getdata.detach();
    while(true)
    {
        //死循环保存主线程不退出
    }
    return 0;
}

int socket_init_bind()//建立最初的套接字，且绑定端口跟ip
{
    int sockfd;
    struct sockaddr_in my_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket is error");
        exit(1);
    }
    cout << "socket is ok" << endl;
    my_addr.sin_family = AF_INET;//ipv4通讯
    my_addr.sin_port = htons(MyPort);//端口
    my_addr.sin_addr.s_addr = inet_addr(MyIp);//ip

    memset(my_addr.sin_zero, 0, sizeof(my_addr.sin_zero));//让sockaddr与sockaddr_in两个数据结构保持相同大小
    if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind is error");
        exit(1);
    }
    cout << "bind is ok" << endl;
    return sockfd;
}

void Conn(int sockfd)
{
    while(true)
    {
        int clientfd;
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = 1;
        if((clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientaddrlen)) == -1)
        {
            perror("accept is error");
            continue;
        }
        if(socknum < Max_Conn_num)
        {
            string msg = "This is ChatRoom!", bits = "";
            Tools::string_to_bits(msg, bits);
            send(clientfd, bits.c_str(), bits.size(), 0);
            char buffer[MaxLen];
            memset(buffer, 0, sizeof(buffer));
            int len = recv(clientfd, buffer, MaxLen, 0);
            if(len < 0)
            {
                perror("recv is error");
                continue;
            }
            buffer[len] = '\0';
            bits = buffer;
            Tools::bits_to_string(bits, msg);
            Client_info client(msg, clientfd);
            g_mutex.lock();//对set操作进行加锁
            client_set.insert(client);
            socknum ++;
            g_mutex.unlock();//解锁
            addEvent(clientfd, EPOLLIN);
            msg += " join the chatroom!";
            Tools::string_to_bits(msg, bits);
            allSend(bits.c_str(), clientfd);
        }
        else 
        {
            string msg = "Sorry,ChatRoom already expired", bits = "";
            Tools::string_to_bits(msg, bits);
            send(clientfd, bits.c_str(), bits.size(), 0);
            close(clientfd);
        }
    }
}

void Get_Data()//处理客户端发送来的数据
{
    while(true)
    {
        int ret = epoll_wait(epollfd, events, Max_Conn_num, -1);
        int clientfd;
        for(int i = 0; i < ret; i ++)
        {
            clientfd = events[i].data.fd;
            if(events[i].events & EPOLLIN)
            {
                char buffer[MaxLen];
                int len = recv(clientfd, buffer, MaxLen, 0);//读取客户端发送来的数据
                if(len <= 0)
                {
                    perror("recv is error");
                    deleteEvent(clientfd, EPOLLIN);
                    for(Client_info it : client_set)
                    {
                        if(it.getclientfd() == clientfd)
                        {
                            g_mutex.lock();
                            client_set.erase(it);
                            socknum --;
                            g_mutex.unlock();
                            break;
                        }
                    }
                }
                else 
                {
                    buffer[len] = '\0';
                    string bits = buffer, message;
                    Tools::bits_to_string(bits, message);
                    if(message == "gln")
                    {
                        allnameSend(clientfd);
                        continue;
                    }
                    vector <string> vs;//存分割后的字符串数组
                    string name;//用户昵称
                    for(auto it : client_set)//根据套接字描述符找到对应的昵称
                    {
                        if(it.getclientfd() == clientfd)
                        {
                            name = it.getname();
                            break;
                        }
                    }
                    Tools::split(message, "@", vs);//分割字符串
                    if(vs.size() == 1)//没有指定用户，故群发
                    {
                        message = name + ":" + message;
                        Tools::string_to_bits(message, bits);
                        allSend(bits.c_str(), clientfd);
                    }
                    else 
                    {
                        message = name + ":" + vs[0];
                        Tools::string_to_bits(message, bits);
                        for(auto it : client_set)//根据昵称找到对应客户端套接字描述符
                        {
                            if(it.getname() == vs[1])
                            {
                                if(send(it.getclientfd(), bits.c_str(), bits.size(), 0) == -1)
                                {
                                    perror("send is error");
                                    deleteEvent(it.getclientfd(), EPOLLIN);
                                    g_mutex.lock();
                                    client_set.erase(it);
                                    socknum --;
                                    g_mutex.unlock();
                                }
                                break;
                            }
                        }

                    }
                }
            }
        }
    }
}

void allSend(const char * buffer, int &clientfd)//群发，对每个当前在线用户都发送消息
{
    if(buffer[0] == '\0')
        return ;
    for(auto it : client_set)
    {
        if(it.getclientfd() == clientfd)//不用发给自己
            continue;
        if(send(it.getclientfd(), buffer, strlen(buffer), 0) == -1)//如果发送失败，删除对应套接字
        {
            perror("send is error");
            deleteEvent(it.getclientfd(), EPOLLIN);//直接删除会导致迭代器失效
            delete_set.push_back(it);
        }
    }
    for(auto it : delete_set)//删除刚才纪录的迭代器
    {
        g_mutex.lock();
        client_set.erase(it);
        socknum --;
        g_mutex.unlock();
    }
    delete_set.clear();
}

void allnameSend(int& clientfd)//获取当前在线人员的名单
{
    string names = "";
    Client_info client;
    for(auto it : client_set)//将所有的昵称整合成一个串
    {
        if(it.getclientfd() == clientfd)
            client = it;
        names += it.getname() + "@";
    }
    if(send(clientfd, names.c_str(), names.size(), 0) == -1)
    {
        perror("send is error");
        deleteEvent(clientfd, EPOLLIN);
        g_mutex.lock();
        client_set.erase(client);   
        socknum --;
        g_mutex.unlock();
    }
}

void addEvent(const int& clientfd, int state)//添加事件
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = clientfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
}

void deleteEvent(const int& clientfd, int state)//删除事件
{
    struct epoll_event ev;
    close(clientfd);
    ev.events = state;
    ev.data.fd = clientfd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, clientfd, &ev);
}

