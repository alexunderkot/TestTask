#include "HttpClient.h"
#include <stdexcept>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

#ifdef _WIN32
HttpClient::WinSockInitializer::WinSockInitializer() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

HttpClient::WinSockInitializer::~WinSockInitializer() {
    WSACleanup();
}
#endif

HttpClient::HttpClient() {
#ifdef _WIN32
    winSock = std::make_unique<WinSockInitializer>();
#endif
}

HttpClient::~HttpClient() = default;

std::string HttpClient::sendRequest(const std::string& host, int port, 
                                   const std::string& request) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Socket creation error");
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    struct hostent* server = gethostbyname(host.c_str());
    if (server == nullptr) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        throw std::runtime_error("Cannot resolve host: " + host);
    }
    
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        throw std::runtime_error("Cannot connect to server");
    }
    
    send(sock, request.c_str(), request.length(), 0);
    
    char buffer[4096];
    std::string response;
    int bytes_read;
    
    while ((bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        response.append(buffer);
    }
    
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    
    return response;
}