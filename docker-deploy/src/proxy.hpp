#ifndef PROXY_HPP
#define PROXY_HPP

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
//#include <mutex>
#include <sstream>

//#include "cache.hpp"
#include "client_info.hpp"
#include "request.hpp"
#include "response.hpp"
#include "server.hpp"

class Proxy {
  const char * port;

 public:
  Proxy() : port(NULL) {}
  Proxy(const char * port) : port(port) {}

  void makeDaemon();
  void runProxy();
  static void * handleRequest(void * args);
  static void handleGET(char * requestmsg,Request req,int server_fd, int client_fd,const char * host, int request_id,int len);
  static void handlePOST(char * requestmsg, Request req,int server_fd, int client_fd,const char * host, int request_id,int len);
  static void handleCONNECT(int server_fd, int client_fd, int request_id);
  static void Send502Error(int fd, int id);
  static void Send400Error(int fd, int id);
};

#endif