#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>
#include <memory>

class IHttpClient {
public:
    virtual ~IHttpClient() = default;
    virtual std::string sendRequest(const std::string& host, int port, 
                                   const std::string& request) = 0;
};

class HttpClient : public IHttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    std::string sendRequest(const std::string& host, int port, 
                           const std::string& request) override;
    
private:
#ifdef _WIN32
    class WinSockInitializer {
    public:
        WinSockInitializer();
        ~WinSockInitializer();
    };
    
    std::unique_ptr<WinSockInitializer> winSock;
#endif
};

#endif