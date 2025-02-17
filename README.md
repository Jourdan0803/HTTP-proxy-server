# HTTP-proxy-server

## A C++ HTTP proxy server with caching and multi-thread, handling 10000+ GET, POST, CONNECT requests.
- Designed LRU cache, reducing API latency by 25%; Ensured thread-safety with lock guards, using RAII and exception handling for
robustness; Provided Docker setup for easy deployment and testing with integrated log access.
