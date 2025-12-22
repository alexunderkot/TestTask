#include "HttpServer.h"
#include "Calculator.h"
#include "JsonParser.h"
#include "SessionStore.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

HttpServer::HttpServer(int port, 
                      std::unique_ptr<ICalculator> calc,
                      std::unique_ptr<IJsonParser> parser,
                      ISessionStore& store)
    : server_fd(0), running(false), port(port),
      calculator(std::move(calc)), jsonParser(std::move(parser)),
      sessionStore(store) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    if (running) return;
    
    initializeSocket();
    
    std::cout << "Calculator Server with Sessions running on port " << port << std::endl;
    std::cout << "Use Ctrl+C to stop" << std::endl;
    
    running = true;
    run();
}

void HttpServer::stop() {
    if (!running) return;
    running = false;
    if (server_fd > 0) {
        close(server_fd);
        server_fd = 0;
    }
}

bool HttpServer::isRunning() const {
    return running;
}

void HttpServer::initializeSocket() {
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        throw std::runtime_error("Socket creation failed");
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        throw std::runtime_error("Socket options failed");
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        throw std::runtime_error("Socket bind failed");
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        throw std::runtime_error("Socket listen failed");
    }
}

void HttpServer::run() {
    while (running) {
        int new_socket;
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            if (running) perror("accept");
            continue;
        }
        
        std::thread([this, new_socket, address]() {
            handleClient(new_socket, address);
        }).detach();
    }
}

void HttpServer::handleClient(int socket, struct sockaddr_in address) {
    char buffer[BUFFER_SIZE] = {0};
    
    ssize_t bytes_read = read(socket, buffer, BUFFER_SIZE - 1);
    if (bytes_read > 0) {
        std::string request(buffer, bytes_read);
        
        std::string response = processHttpRequest(request);
        
        send(socket, response.c_str(), response.length(), 0);
    }
    
    close(socket);
}

std::string HttpServer::processHttpRequest(const std::string& request) {
    size_t body_start = request.find("\r\n\r\n");
    if (body_start == std::string::npos) {
        return createHttpResponse(jsonParser->createError("Invalid HTTP request"));
    }
    
    std::string body = request.substr(body_start + 4);
    
    std::string exp, cmd, user;
    if (!jsonParser->parseRequest(body, exp, cmd, user)) {
        return createHttpResponse(jsonParser->createError("Invalid JSON"));
    }
    
    if (!cmd.empty()) {
        if (cmd == "echo") {
            return createHttpResponse(jsonParser->createResponse("echo"));
        } else if (cmd == "clean") {
            std::string effectiveUser = sessionStore.getEffectiveUser(user);
            sessionStore.clear(effectiveUser);
            return createHttpResponse(jsonParser->createEmptyResponse());
        } else {
            return createHttpResponse(jsonParser->createError("Unknown command"));
        }
    }
    
    if (!exp.empty()) {
        try {
            std::string result = calculator->evaluateExpression(exp, user);
            if (result.empty()) {
                return createHttpResponse(jsonParser->createEmptyResponse());
            } else {
                try {
                    if (result.find(';') == std::string::npos) {
                        double numResult = std::stod(result);
                        return createHttpResponse(jsonParser->createResponse(numResult));
                    } else {
                        return createHttpResponse(jsonParser->createResponse(result));
                    }
                } catch (...) {
                    return createHttpResponse(jsonParser->createResponse(result));
                }
            }
        } catch (const std::exception& e) {
            return createHttpResponse(jsonParser->createError(e.what()));
        }
    }
    
    return createHttpResponse(jsonParser->createError("No expression provided"));
}

std::string HttpServer::createHttpResponse(const std::string& json_body) {
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + std::to_string(json_body.length()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        json_body;
    
    return response;
}