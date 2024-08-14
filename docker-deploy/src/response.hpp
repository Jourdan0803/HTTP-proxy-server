#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <iomanip>

using namespace std;

class Response{
public:
    std::string responseContent;
    std::string line;
    std::string status;
    std::map<std::string, std::string> headers;
    std::string eTag;
    std::string lastModified;
    size_t contentLength;
    std::string expirationTime;
    std::string date;

    string receivedTime;

    size_t maxAge;
    size_t maxStale;
    bool mustRevalidate = false;
    bool cacheControl = false;
    bool privateCache = false;
    bool isChunked = false;
    bool noCache = false;
    bool noStore = false;
    bool hasMaxAge = false;
    bool hasMaxStale = false;

    Response(){}
    Response(const std::string & responseContent):responseContent(responseContent){parseResponse();}
    
    void parseResponse();
    void parseStatusLine(std::istringstream& stream);
    void parseCacheControl(const std::string& cacheControlValue);
    void parseHeaders(std::istringstream& stream);
    void parseExpireDate();
    string getCurrTime();
};
#endif