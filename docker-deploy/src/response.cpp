#include "response.hpp"
#include <sstream>
#include <algorithm>
// Trim from start (left)
static inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return s;
}

// Trim from end (right)
static inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

// Trim from both ends
static inline std::string& trim(std::string& s) {
    return ltrim(rtrim(s));
}

void Response::parseResponse() {
    this->receivedTime = this->getCurrTime();
    std::istringstream stream(responseContent);
    parseStatusLine(stream);
    parseHeaders(stream);
}

void Response::parseStatusLine(std::istringstream& stream) {
    std::getline(stream, line);
    size_t lineEnd = line.find("\r");
    if (lineEnd != string::npos) {
        line = line.substr(0, lineEnd);
    }
    size_t pos = line.find(' ');
    if (pos != std::string::npos) {
        status = line.substr(pos + 1);
    }
}

void Response::parseCacheControl(const std::string& cacheControlValue) {
    std::istringstream cacheControlStream(cacheControlValue);
    std::string directive;
    while (std::getline(cacheControlStream, directive, ',')) {
        std::istringstream directiveStream(directive);
        size_t equalPos = directive.find('=');
        std::string key = directive.substr(0, equalPos);
        std::string value;
        if(equalPos != std::string::npos){
            value = directive.substr(equalPos + 1,equalPos + 2);
        }
        //std::getline(directiveStream, value);

        if (trim(key) == "max-age") {
            maxAge = stoul(value);
            hasMaxAge = true;
        } else if (trim(key) == "max-stale") {
            maxStale = stoul(value);
            hasMaxStale = true;
        } else if (trim(key) == "must-revalidate") {
            mustRevalidate = true;
        } else if (trim(key) == "private") {
            privateCache = true;
        } else if (trim(key) == "no-cache") {
            noCache = true;
        } else if (trim(key) == "no-store") {
            noStore = true;
        }
    }
}

void Response::parseHeaders(std::istringstream& stream) {
    std::string line;
    while (std::getline(stream, line) && !line.empty() && line != "\r\n") {
        if (line.find("chunked") != std::string::npos){
            isChunked = true;
        }
        size_t delimiterPos = line.find(": ");
        if (line.find("ETag: ") != std::string::npos){
            eTag = line.substr(delimiterPos + 2);
        }
        if (line.find(": ") != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            headers[key] = value;
            auto cacheControlIter = headers.find("Cache-Control");
            if (cacheControlIter != headers.end()) {
                parseCacheControl(cacheControlIter->second);
            } else if (key == "Last-Modified") {
                lastModified = value;
            } else if (key == "Date") {
                date = value;
            } else if (key == "Expires") {
                expirationTime = value;
            } else if (key == "Content-Length") {
                contentLength = stoul(value);
            }
        }
    }
}

string Response::getCurrTime() {
    time_t curr = time(nullptr);
    tm * utc_curr = gmtime(&curr);
    string utc_now = asctime(utc_curr);
    return utc_now;
}

