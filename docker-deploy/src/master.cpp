#include "server.hpp"
using namespace std;
int main(int argc, char **argv){
    const char* port = argv[1]; 
    Master master(port);
    std::string ip = "";
    //std::cout << "master port"<< argv[1]<< endl;
    master.createServer();
    int fd =  master.acceptClient(ip);
    // std::cout << "client fd:"<< fd;
    // std::cout << "client ip:"<< ip;
    //std::cout << master.ip_list[0];
    return 1;
}