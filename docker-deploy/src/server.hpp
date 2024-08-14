#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream> // 引入标准输入输出流库
#include <fstream>
#include <cstring> // 引入字符串处理库
#include <sys/socket.h> // 引入套接字接口
#include <netdb.h> // 引入网络数据库操作库
#include <unistd.h> // 引入POSIX操作系统API
#include <arpa/inet.h>
#include <string>
#include <vector>

class Server {
    public:
        int status;
        int socket_fd; // 套接字文件描述符
        struct addrinfo host_info;// 存储主机信息的结构
        struct addrinfo *host_info_list; // 指向addrinfo结构的指针
        const char * hostname;
        const char * port;
    public:
        Server();
        Server(const char * port);
        Server(const char *hostname, const char *port) : hostname(hostname), port(port) {}
        virtual  ~Server();
        virtual void init_addrinfo();
        // virtual void createSocket();
        virtual int createSocket();
        void printHostname() { std::cout << "hostname: " << hostname << std::endl; };
    };
class Master : public Server{
    public:
        Master(const char * port) : Server(port) {}
        void listenForClients();
        int acceptClient(std::string & ip);
        void createServer();
};
class Client : public Server {
    public:
        Client(const char * hostname,const char * port): Server(hostname,port) {}
        void init_addrinfo() override;
        void connectToServer();
        // void createClient();
};
#endif  // SERVER_HPP