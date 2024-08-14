#include <pthread.h>
#include "proxy.hpp"
int main() { 
  // testing for docker container linking comments
  // cout << "testing for docker container linking comments" << endl;
  const char * port = "12345";
  Proxy * myproxy = new Proxy(port);
  myproxy->runProxy();
  return 1;
}