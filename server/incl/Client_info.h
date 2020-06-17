/*管理客户端连接信息*/
#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H
#include <string>
class Client_info
{
    private:
        std::string virname;//客户端用户昵称
        int client_fd;//客户段套接字id
    public:
        //默认构造函数
        Client_info(std::string name = "", int fd = 0);
        //拷贝构造函数
        Client_info(const Client_info& p);
        //析构函数
        ~Client_info();
        std::string getname();
        int getclientfd();
        //初始化
        void init();
        //重载赋值运算符
        Client_info& operator = (const Client_info& p);
        bool operator < (const Client_info& p) const;
        bool operator > (const Client_info& p) const;
};
#endif
