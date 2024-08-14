#include "server.hpp"
using namespace std;

// Server implementation
Server::Server() : hostname(NULL), port(NULL), socket_fd(-1), host_info_list(NULL) {}

Server::Server(const char *port) : hostname(NULL), port(port), socket_fd(-1), host_info_list(NULL) {}

Server::~Server() {
    if (host_info_list) free(host_info_list);
    if (socket_fd != -1) close(socket_fd);
}
void Server::init_addrinfo() {
    memset(&host_info, 0, sizeof(host_info));// 初始化host_info结构

    host_info.ai_family   = AF_UNSPEC; // 设置地址族为未指定
    host_info.ai_socktype = SOCK_STREAM; // 设置套接字类型为流式套接字
    host_info.ai_flags    = AI_PASSIVE; // 套接字用于被动监听
}
int Server::createSocket() {
    status = getaddrinfo(hostname, port, &host_info, &host_info_list);// 获取地址信息
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "hostname: " << hostname << endl;
        cerr << "port: " << port << endl;
        // exit(EXIT_FAILURE);
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);// 创建套接字
    //cout << "server.cpp line 36: socket_fd: "<< socket_fd << endl;
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        // exit(EXIT_FAILURE);
        return -2;
    } //if
    return 0;
}

void Client::init_addrinfo() {
    memset(&host_info, 0, sizeof(host_info));// 初始化host_info结构

    host_info.ai_family   = AF_UNSPEC; // 设置地址族为未指定
    host_info.ai_socktype = SOCK_STREAM; // 设置套接字类型为流式套接字
}

void Client::connectToServer() {
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen); // 连接到服务器
    if (status == -1) {
        cerr << socket_fd << endl;
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        //exit(EXIT_FAILURE);
    } //if
}
void Master::listenForClients() {
    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // 设置套接字选项
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen); // 绑定套接字到端口
    if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        //exit(EXIT_FAILURE);
    } //if

    status = listen(socket_fd, 100); // 监听端口
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl; 
        cerr << "  (" << hostname << "," << port << ")" << endl;
       //exit(EXIT_FAILURE);
    } //if
}
int Master::acceptClient(std::string & ip){
    //cout << "enter Master::acceptClient()" << endl;
    struct sockaddr_storage socket_addr; // 存储套接字地址的结构
    socklen_t socket_addr_len = sizeof(socket_addr);//客户端地址数据
    int client_connection_fd; // 客户端连接的文件描述符
    char ips[1024];
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);// 接受客户端连接
    if (client_connection_fd == -1) {
        std::cerr << "Error: cannot accept connection on socket" << std::endl;
        //exit(EXIT_FAILURE);
    } //if
    struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
    strcpy(ips, inet_ntoa(addr->sin_addr));//获取client ip
    // ip_list.push_back(ip); 
    // fd_list.push_back(client_connection_fd);
    // return client_connection_fd;
    ip = ips;
    return client_connection_fd;
}
void Master::createServer(){
    //cout << ((hostname == NULL) ? "Master::createServer() host is NULL" : "Master::createServer() not null") << endl;
    init_addrinfo();
    createSocket();
    listenForClients();
    //cout << ((hostname == NULL) ? "Master::createServer() host is NULL" : "Master::createServer() host not null") << endl;
}
