// #include <sstream>
#include "request.hpp"

using namespace std;

long Request::nextRequestID = 1;

void Request::readRequest() {
    // first generate the received time
    this->receivedTime = this->getCurrTime();
    
    stringstream ss(this->requestContent);
    string line;
    if (getline(ss, line)) {
        size_t lineEnd = line.find("\r");
        if (lineEnd != string::npos) {
            line = line.substr(0, lineEnd);
        } else {
        //     // 如果没有找到 "\r\n"，则检查 "\n"
            lineEnd = line.find("\n");
            if (lineEnd != string::npos) {
                line = line.substr(0, lineEnd);
            }
        }
        // size_t linekey = line.find("\r\n");
        this->requestLine = line;
        istringstream lineStream(line);
        lineStream >> this->method >> this->url;

        if (this->method != "POST" && this->method != "GET" && this->method != "CONNECT") {
            cerr << "The method " <<  this->method << " is not supported." << endl;
        }
    }
    
    while (getline(ss, line) && line != "\r") {
        auto colon = line.find(':');
        // xx : xx
        if (colon != string::npos) {
            string headerKey = line.substr(0, colon);
            string headerValue = line.substr(colon + 2);
            // this->headers[headerKey] = headerValue;
            this->headers[headerKey] = headerValue.erase(headerValue.find_last_not_of("\r\n") + 1);
        }
    }

    string host_port = this->headers["Host"];
    size_t portSplit = host_port.find(':');
    if (portSplit != string::npos) {
        this->hostname = host_port.substr(0, portSplit);
        // this->hostname += '\0';
        this->port = host_port.substr(portSplit + 1);
        //this->port += '\0';
    }
    else {
        this->hostname = host_port;
        // this->hostname += '\0';
        // this->port = "12345";
        this->port = "80";
    }
    this->hostname.erase(this->hostname.find_last_not_of("\r\n") + 1);

    nextRequestID++;
}

string Request::getCurrTime() {
    time_t curr = time(nullptr);
    tm * utc_curr = gmtime(&curr);
    string utc_now = asctime(utc_curr);
    return utc_now;
}

string Request::getMethod() {
    return this->method;
}

string Request::getRequestLine() {
    return this->requestLine;
}

string Request::getURL() {
    return this->url;
}

string Request::getHost() {
    return this->hostname;
}

string Request::getPort() {
    return this->port;
}

string Request::getIPFrom() {
    return this->IPfrom;
}

string Request::getReceivedTime() {
    return this->receivedTime;
}

long Request::getID() {
    return this->requestID;
}

void Request::printRequest() {
    // cout << "requestContent: " << this->requestContent << "checkSpace" << endl;
    // cout << "RequestID: " << this->requestID << "checkSpace" << endl;
    // cout << "nextRequestID: " << this->nextRequestID << "checkSpace" << endl;
    // cout << "Method: " << this->method << "checkSpace" << endl;
    // cout << "requestLine: " << this->requestLine << "checkSpace" << endl;
    // cout << "url: " << this->url << "checkSpace" << endl;
    // cout << "hostname: " << this->hostname << "checkSpace" << endl;
    // cout << "port: " << this->port << "checkSpace" << endl;
    // cout << "IPfrom: " << this->IPfrom << "checkSpace" << endl;
    // cout << "receivedTime: " << this->receivedTime << "checkSpace" << endl;
}