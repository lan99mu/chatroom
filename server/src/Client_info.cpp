#include "Client_info.h"
//默认构造函数
Client_info::Client_info(std::string name, int fd) : virname(name), client_fd(fd) 
{

}
//拷贝构造函数
Client_info::Client_info(const Client_info& p)
{
    this->virname = p.virname;
    this->client_fd = p.client_fd;
}
//析构函数
Client_info::~Client_info() 
{

}
//重载赋值运算符
Client_info& Client_info::operator = (const Client_info& p)
{
    this->virname = p.virname;
    this->client_fd = p.client_fd;
    return *this;
}
//初始化
void Client_info::init()
{
    this->virname = "";
    this->client_fd = 0;
}

std::string Client_info::getname()
{
    return this->virname;
}

int Client_info::getclientfd()
{
    return this->client_fd;
}

bool Client_info::operator < (const Client_info& p) const
{
    return client_fd < p.client_fd;
}

bool Client_info::operator > (const Client_info& p) const
{
    return client_fd > p.client_fd;
}
