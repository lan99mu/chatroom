#include "Tools.h"
#include "Client_info.h"
#include <bits/stdc++.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

int socket_init_bind();
void Conn(int);
void Get_Data();
void allSend(const char *, int&);
void addEvent(int clientfd, int state);
void deleteEvent(int clientfd, int state);


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
            char buffer[MaxLen] = "this is chatroom";
            send(clientfd, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            int len = recv(clientfd, buffer, MaxLen, 0);
            if(len < 0)
            {
                perror("recv is error");
                continue;
            }
            buffer[len] = '\0';
            string name = buffer;
            Client_info client(name, clientfd);
            client_set.insert(client);
            socknum ++;
            addEvent(clientfd, EPOLLIN);
            strcat(buffer, "join the chatroom!");
            allSend(buffer, clientfd);
        }
        else 
        {
            char buffer[] = "Sorry,chatroom already expired\n";
            send(clientfd, buffer, strlen(buffer), 0);
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
                            client_set.erase(it);
                            break;
                        }
                    }
                }
                else 
                {
                    buffer[len] = '\0';
                    string message = buffer;
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
                        allSend(message.c_str(), clientfd);
                    }
                    else 
                    {
                        message = name + ":" + vs[0];
                        for(auto it : client_set)//根据昵称找到对应客户端套接字描述符
                        {
                            if(it.getname() == vs[1])
                            {
                                if(send(it.getclientfd(), message.c_str(), message.size(), 0) == -1)
                                {
                                    perror("send is error");
                                    deleteEvent(it.getclientfd(), EPOLLIN);
                                    client_set.erase(it);
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

void allSend(const char * buffer, int &clientfd)
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
        client_set.erase(it);
    delete_set.clear();
}

void addEvent(int clientfd, int state)//添加事件
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = clientfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
}

void deleteEvent(int clientfd, int state)//删除事件
{
    struct epoll_event ev;
    close(clientfd);
    ev.events = state;
    ev.data.fd = clientfd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, clientfd, &ev);
}

