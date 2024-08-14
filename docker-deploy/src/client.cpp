#include "server.hpp"
using namespace std;
int main(int argc, char **argv){
    const char* port = argv[1]; 
    const char* hostname = argv[2]; 
    Client client(hostname,port);
    client.init_addrinfo();
    client.createSocket();
    client.connectToServer();
    cout << "Client connected to server " << endl;
    return 1;
}