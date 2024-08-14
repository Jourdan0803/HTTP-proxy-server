#include "proxy.hpp"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <ctime>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>

#include "client_info.hpp"
#include "cache.hpp"
using namespace std;

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;
// for local run
// std::ofstream logFile("../var/log/erss/proxy.log");
// for docker run
std::ofstream logFile("/var/log/erss/proxy.log");

Cache cache(logFile);

bool isZeroLengthChunk(const char* chunk, int length) {
    // A zero-length chunk is just "0\r\n"
    return length == 3 && chunk[0] == '0' && chunk[1] == '\r' && chunk[2] == '\n';
}
void Proxy::runProxy(){
    //cout << "runProxy()" << endl;
    Master proxyServer(port);
    proxyServer.createServer();
    if(proxyServer.status == -1){
        pthread_mutex_lock(&mymutex);
        logFile << "(no-id): ERROR Cannot create server" << std::endl;
        pthread_mutex_unlock(&mymutex);
        return;
    }
    int request_id = 0;//*********************

    while (true) {
        std::string client_ip = "";
        int client_fd= proxyServer.acceptClient(client_ip);
        if (client_fd == -1) {
            pthread_mutex_lock(&mymutex);
            logFile << "(no-id): ERROR Cannot accept connection from client" << std::endl;
            pthread_mutex_unlock(&mymutex);
            continue;
        }

        pthread_t thread;
        pthread_mutex_lock(&mymutex);
        ClientInfo clientinfo;
        clientinfo.client_fd = client_fd;
        clientinfo.client_ip = client_ip;
        clientinfo.request_id = request_id;
        request_id++;
        pthread_mutex_unlock(&mymutex);
        pthread_create(&thread, NULL, handleRequest, &clientinfo);
    }
}

void * Proxy::handleRequest(void * args){
    // get client info
    ClientInfo * clientinfo = (ClientInfo *)args;
    int client_fd = clientinfo->client_fd;
    int request_id = clientinfo->request_id;
    std::string client_ip = clientinfo->client_ip;

    char requestmsg[65536] = {0};
    int len = recv(client_fd, requestmsg, sizeof(requestmsg), 0);  // fisrt request from client
    if (len <= 0) {
        // pthread_mutex_lock(&mymutex);
        // logFile << request_id << ": WARNING Invalid Request" << std::endl;
        // pthread_mutex_unlock(&mymutex);
        close(client_fd);
        return NULL;
    }
    //convert massage to string
    std::string megstr = std::string(requestmsg, len);
    // if (megstr == "" || megstr == "\r" || megstr == "\n" || megstr == "\r\n") {
    //     return NULL;}
    Request req(megstr);
    // req.printRequest();
    if (req.getMethod() != "POST" && req.getMethod() != "GET" && req.getMethod() != "CONNECT") {
        Send400Error(client_fd,request_id);
    }
    pthread_mutex_lock(&mymutex);
    logFile << request_id << ": \"" << req.getRequestLine() << "\" from " << client_ip << " @ " << req.getCurrTime();
    pthread_mutex_unlock(&mymutex);

    //std::cout << "received client request is:" << req_msg << "end" << std ::endl;
    string hostname_str = req.getHost();
    // const char * hostname = req.getHost().c_str();
    const char * hostname = hostname_str.c_str();
    const char * port = req.getPort().c_str();
    Client client(hostname,port); //connect to server
    client.init_addrinfo();
    // client.printHostname();
    int intCreateSocket = client.createSocket();

    if (intCreateSocket == -1) {
        pthread_mutex_lock(&mymutex);
        logFile << request_id << "ERROR Cannot get address info for host" << std::endl;
        pthread_mutex_unlock(&mymutex);
        return NULL;
    }
    if (intCreateSocket == -2) {
        pthread_mutex_lock(&mymutex);
        logFile << request_id <<  "ERROR Cannot create socket" << std::endl;
        pthread_mutex_unlock(&mymutex);
        return NULL;
    }
    
    //cout << "begin connectToServer()" << endl;
    client.connectToServer();
    //cout << "exit connectToServer()" << endl;

    if (client.socket_fd == -1) {
        logFile << request_id << ": ERROR Cannot connect to socket" << std::endl;
        return NULL;
    }
    if (req.getMethod() == "CONNECT") {
        pthread_mutex_lock(&mymutex);
        logFile << request_id << ": " << "Requesting \"" << req.getRequestLine() << "\" from " << hostname << std::endl;
        pthread_mutex_unlock(&mymutex);
        handleCONNECT(client.socket_fd, client_fd, clientinfo->request_id);
        pthread_mutex_lock(&mymutex);
        logFile << request_id << ": Tunnel closed" << std::endl;
        pthread_mutex_unlock(&mymutex);
    }
    else if (req.method == "GET") {
        handleGET(requestmsg,req,client.socket_fd, client_fd, hostname, request_id,len);
    }
    else if (req.method == "POST") {
        handlePOST(requestmsg,req, client.socket_fd, client_fd, hostname, request_id,len);
    }
    close(client.socket_fd);
    close(client_fd);
    return NULL;
}

void Proxy::handleGET(char * requestmsg,Request req, int server_fd, int client_fd,const char * host, int request_id,int len){
    // if in cache
    //map<std::string, Response>::const_iterator it = cache.cacheResponse.begin();
    map<std::string, Response>::const_iterator it = cache.cacheResponse.find(req.getURL());
    if (it != cache.cacheResponse.end()) {
        //cout << "res in cache" << endl;
        Response res = *(cache.getResponse(req, requestmsg, len, server_fd, request_id));//********************************************//
        int num = send(client_fd,res.responseContent.c_str(),res.responseContent.length(),0);

        pthread_mutex_lock(&mymutex);
        logFile << request_id << ": Responding \"" << res.line << "\"" << std::endl;
        pthread_mutex_unlock(&mymutex);
        close(server_fd);
        return;
    }else{
        //if not in cache
        pthread_mutex_lock(&mymutex);
        logFile << request_id << ": not in cache" << std::endl;
        pthread_mutex_unlock(&mymutex);
        pthread_mutex_lock(&mymutex);
        logFile << request_id << ": "<< "Requesting \"" << req.getRequestLine() << "\" from " << host << std::endl;
        pthread_mutex_unlock(&mymutex);
        //send request to server as a client
        send(server_fd, requestmsg, len, 0);
        //receive response from server
        char servermsg[65536] = {0};
        int lenServer = recv(server_fd, servermsg, sizeof(servermsg), 0);  // fisrt request from server
        //cout<<servermsg<<endl;
        if (lenServer == -1) {
            Send502Error(client_fd, request_id);
            close(server_fd);
            return;
        }
        
        //parse response from server
        Response res(servermsg);
        // cout << "content response" << servermsg<<endl;
        // cout << "content etag" << res.eTag<<endl;
        pthread_mutex_lock(&mymutex);
        logFile << request_id << ": Received \"" << res.line << "\" from " << host << std::endl;
        pthread_mutex_unlock(&mymutex);
        bool chunk = res.isChunked;
        if(chunk){
            //cout << "chuck" << endl;
            //if chunk, send message by chunk
            // pthread_mutex_lock(&mymutex);
            // logFile << request_id << ": not cacheable because chunked" << std::endl;
            // pthread_mutex_unlock(&mymutex);
            //send response to client
            send(client_fd, servermsg, lenServer, 0);
            char chunkedmsg[65036] = {0};
            int lenChunk;
            std::string completeMessage;
            completeMessage.append(servermsg,lenServer);
            while (true) {  //receive and send remaining message
                memset(chunkedmsg, 0, sizeof(chunkedmsg)); // Clear the buffer
                lenChunk = recv(server_fd, chunkedmsg, sizeof(chunkedmsg), 0);
                //cout << lenChunk <<endl;
                if (lenChunk == -1 || lenChunk == 5) {
                    // Handle error
                    //cout << "break" << endl;
                    break;
                } else if (lenChunk == 0) {
                    // Check for zero-length chunk here
                if (isZeroLengthChunk(chunkedmsg,lenChunk)) {
                    break;}
                }else {
                    send(client_fd, chunkedmsg, lenChunk, 0);
                    completeMessage.append(chunkedmsg, lenChunk);
                }
                
            }
            Response chunkRes(completeMessage);
            cache.addResponse(req,chunkRes,request_id);
            //cout << "exit while chunk loop" << endl;
            //send(client_fd, servermsg, lenChunk, 0);
        }else{
            //if not chunk, add res to cache
            send(client_fd, servermsg, lenServer, 0);
            pthread_mutex_lock(&mymutex);
            //cout << "not chunk" << endl;
            cache.addResponse(req, res, request_id);//********************************************//
            //cout << "not chunk add response into cache done" << endl;
            pthread_mutex_unlock(&mymutex);
        }
        pthread_mutex_lock(&mymutex);
        logFile << request_id << ": Responding \"" << res.line << "\""<< std::endl;
        pthread_mutex_unlock(&mymutex);
    }
    close(server_fd);
    //cout << "close server fd" << endl;
    //cout << "exit handleGET()" << endl;
}

void Proxy::handlePOST(char * requestmsg, Request req, int server_fd, int client_fd,const char * host, int request_id,int len){
    //cout << "enter handlePOST()" << endl;
    int req_len = send(server_fd, requestmsg, len, 0);
    pthread_mutex_lock(&mymutex);
    logFile << request_id << ": Requesting \"" << req.getRequestLine() << "\" from "
            << host << std::endl;
    pthread_mutex_unlock(&mymutex);
    char servermsg[65536] = {0};
    int lenServer = recv(server_fd, servermsg, sizeof(servermsg), 0);  // fisrt request from server
    if (lenServer == -1) {
        Send502Error(client_fd, request_id);
        close(server_fd);
        return;
    }
    Response res(servermsg);
    pthread_mutex_lock(&mymutex);
    logFile << request_id << ": Received \"" << res.line << "\" from " << host << std::endl;
    pthread_mutex_unlock(&mymutex);
    if(res.isChunked==true){
        //if chunk, send message by chunk
            //cout << "chuck" << endl;
            //if chunk, send message by chunk
            // pthread_mutex_lock(&mymutex);
            // logFile << request_id << ": not cacheable because chunked" << std::endl;
            // pthread_mutex_unlock(&mymutex);
            //send response to client
            send(client_fd, servermsg, lenServer, 0);
            char chunkedmsg[65036] = {0};
            int lenChunk;
            std::string completeMessage;
            while (true) {  //receive and send remaining message
                memset(chunkedmsg, 0, sizeof(chunkedmsg)); // Clear the buffer
                lenChunk = recv(server_fd, chunkedmsg, sizeof(chunkedmsg), 0);
                //cout << lenChunk <<endl;
                if (lenChunk == -1||lenChunk==-5) {
                    // Handle error
                    break;
                } else if (lenChunk == 0) {
                    // Check for zero-length chunk here
                if (isZeroLengthChunk(chunkedmsg,lenChunk)) {
                    // std::cout << "End of chunked message\n";
                    break;}
                }else {
                    send(client_fd, chunkedmsg, lenChunk, 0);
                    completeMessage.append(chunkedmsg, lenChunk);
                }
                
            }
        //cout << "exit while chunk loop" << endl;
        //send(client_fd, servermsg, lenChunk, 0);
        //send(client_fd, servermsg, lenChunk, 0);
}else{
        //if not chunk, add res to cache
        send(client_fd, servermsg, lenServer, 0);
        pthread_mutex_lock(&mymutex);
        pthread_mutex_unlock(&mymutex);
    }
    pthread_mutex_lock(&mymutex);
    logFile << request_id << ": Responding \"" << res.line << "\""<< std::endl;
    pthread_mutex_unlock(&mymutex);
    close(server_fd);
    //cout << "close fd and exit handlePOST()" << endl;
}

void Proxy::handleCONNECT(int server_fd, int client_fd, int request_id){
    //cout << "enter handleCONNECT()" << endl;
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    pthread_mutex_lock(&mymutex);
    logFile << request_id << ": Responding \"" << "HTTP/1.1 200 OK\"" << std::endl;
    pthread_mutex_unlock(&mymutex);
    fd_set readfds;
    int nfds = max(server_fd, client_fd);
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        FD_SET(client_fd, &readfds);
        
        select(nfds+1, &readfds, NULL, NULL, NULL);
        for (int i = 0; i < 2; i++) {
            char buffer[65536] = {0};
            int fd = i == 0 ? server_fd : client_fd;
            int other_fd = i == 0 ? client_fd : server_fd;

            if (FD_ISSET(fd, &readfds)) {
                int len = recv(fd, buffer, sizeof(buffer), 0);
                if (len < 0) {
                    return;
                } else if (len == 0) {
                    //cout << "Socket closed" << endl;
                    return;
                }
                if (send(other_fd, buffer, len, 0) < 0) {
                    return;
                }//cout << "send successful" <<endl;
            }
        }
    }
    //cout << "exit handleCONNECT()" << endl;
}

void Proxy::Send502Error(int client_fd, int request_id) {
  if (send(client_fd, "HTTP/1.1 502 Bad Gateway\r\n\r\n", 28, 0) == -1) {
    pthread_mutex_lock(&mymutex);
    logFile << request_id << ": ERROR Send 502 failed!" << std::endl;
    pthread_mutex_unlock(&mymutex);
    return;
  }
}

void Proxy::Send400Error(int client_fd, int request_id) {
  if (send(client_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 28, 0) == -1) {
    pthread_mutex_lock(&mymutex);
    logFile << request_id << ": ERROR Send 400 failed!" << std::endl;
    pthread_mutex_unlock(&mymutex);
    return;
  }
}