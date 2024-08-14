#include "response.hpp"
#include <iostream>
#include <string>

int main() {
    // Simulate an HTTP response string
    std::string httpResponse = 
        "HTTP/1.1 200 OK\r\n"
        "Expires: Wed, 22 Feb 2024 12:28:53 GMT\r\n"
        "Date: Wed, 21 Feb 2024 12:28:53 GMT\r\n"
        "Server: Apache/2.4.1 (Unix)\r\n"
        "Last-Modified: Wed, 22 Feb 2024 11:18:35 GMT\r\n"
        "ETag: \"3f80f-1b6-3e1cb03b\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 438\r\n"
        "Cache-Control: private, max-age=0, no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n"
        "Content-Type: text/html\r\n"
        "Connection: Closed\r\n\r\n"
        "<html>\r\n"
        "<body>\r\n"
        "<h1>Hello, World!</h1>\r\n"
        "</body>\r\n"
        "</html>";

    // Create a Response object and parse the HTTP response
    Response response(httpResponse);
    response.parseResponse();

    // Output the parsed data for verification
    std::cout << "Status: " << response.status << std::endl;
    std::cout << "ETag: " << response.eTag << std::endl;
    std::cout << "Last Modified: " << response.lastModified << std::endl;
    std::cout << "Content Length: " << response.contentLength << std::endl;
    std::cout << "Max Age: " << response.maxAge << std::endl;
    std::cout << "has max age: " << (response.hasMaxAge? "Yes" : "No") << std::endl;
    std::cout << "cache: " << (response.noCache? "Yes" : "No") << std::endl;
    std::cout << "store: " << (response.noStore? "Yes" : "No") << std::endl;
    std::cout << "private: " << (response.privateCache ? "Yes" : "No")<< std::endl;
    std::cout << "max stale: " << response.maxStale << std::endl;
    std::cout << "has max stale: " << (response.hasMaxStale? "Yes" : "No") << std::endl;
    std::cout << "Must Revalidate: " << (response.mustRevalidate ? "Yes" : "No") << std::endl;
    std::cout << "Expires:" << response.expirationTime << std::endl;
    std::cout << "Chunked: "<< (response.isChunked?"Yes":"No") << std::endl;
    std::cout << "Date:" << response.date << std::endl;
    std::cout << "Line:" << response.line << std::endl;
    //g++ -o responseTest response.cpp testResponse.cpp
    //./responseTest  
    return 0;
}
