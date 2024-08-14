#ifndef CLIENTINFO_HPP
#define CLIENTINFO_HPP

#include <string>

typedef struct ClientInfo_t {
  int client_fd;
  std::string client_ip;
  int request_id;
} ClientInfo;

#endif