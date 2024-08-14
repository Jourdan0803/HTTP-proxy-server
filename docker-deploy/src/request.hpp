#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <ctime>


using namespace std;

class Request {
 public:
    // remember to initalize the requestID in the global scope
    // example: long Request::nextRequestID = 1;
    long requestID;
    static long nextRequestID;

    string requestContent;
    string method;
    //string status;
    map<string, string> headers;
    string requestLine;
    string url;
    string hostname;
    string port;
    string IPfrom;
    string receivedTime;

    Request(string _request) : requestContent(_request), requestID(nextRequestID)  { readRequest(); };
    void readRequest();
    string getCurrTime();
    string getMethod();
    //string getStatus();
    string getRequestLine();
    string getURL();
    string getHost();
    string getPort();
    string getIPFrom();
    string getReceivedTime();
    long getID();

    // for test
    void printRequest();
};

#endif