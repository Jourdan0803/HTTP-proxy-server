#ifndef CACHE_HPP
#define CACHE_HPP

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <ctime>
#include <mutex>

#include "response.hpp"
#include "request.hpp"
#include "server.hpp"

using namespace std;

class Cache {
 public:
    map<string, Response> cacheResponse;
    mutex cacheMutex;
    std::ofstream& log;
    list<string> fifoCache;

    Cache(std::ofstream & _log) : log(_log) {};

    void addResponse(Request req, Response res, int request_id);
    // bool checkReponse(Request req, Response res);
    Response * getResponse(Request req, char * requestmsg, int len, int fd, int request_id);
    bool isExpired(const std::string expireTime,
                    const std::string receiveTime,
                    const size_t maxAge);
};

#endif