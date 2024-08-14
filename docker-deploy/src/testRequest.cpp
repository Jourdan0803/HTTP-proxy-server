#include "request.hpp"
#include <iostream>
#include <string>

using namespace std;


int main() {
    // Simulate an HTTP response string
    string testGetRequest = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Connection: close\r\n"
        "\r\n";

    string testConnectRequest =
        "CONNECT test.com:443 HTTP/1.1\r\n"
        "Host: test.com:443\r\n"
        "\r\n";

    string testPostRequest =
        "POST /submit HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 27\r\n"
        "\r\n"
        "field1=value1&field2=value2";
    
    Request reqGet(testGetRequest);
    reqGet.printRequest();

    Request reqConnect(testConnectRequest);
    reqConnect.printRequest();

    Request reqPost(testPostRequest);
    reqPost.printRequest();

    string test = 
        "CONNECT mozilla.cloudflare-dns.com:443 HTTP/1.1\r\n"
        "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:123.0) Gecko/20100101 Firefox/123.0\r\n"
        "Proxy-Connection: keep-alive\r\n"
        "Connection: keep-alive\r\n"
        "Host: mozilla.cloudflare-dns.com:443\r\n"
        "\r\n";
    Request testReq(test);
    testReq.printRequest();

    // g++ -o testRequest testRequest.cpp request.cpp
    // ./testRequest

    return 0;
}
