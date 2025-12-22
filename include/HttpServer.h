#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

class ICalculator;
class IJsonParser;
class ISessionStore;

class HttpServer {
private:
    int server_fd;
    std::atomic<bool> running;
    int port;
    
    std::unique_ptr<ICalculator> calculator;
    std::unique_ptr<IJsonParser> jsonParser;
    ISessionStore& sessionStore;
    
public:
    HttpServer(int port, 
              std::unique_ptr<ICalculator> calc,
              std::unique_ptr<IJsonParser> parser,
              ISessionStore& store);
    ~HttpServer();
    
    void start();
    void stop();
    bool isRunning() const;
    
private:
    void run();
    void handleClient(int socket, struct sockaddr_in address);
    std::string processHttpRequest(const std::string& request);
    std::string createHttpResponse(const std::string& json_body);
    void initializeSocket();
};

#endif