#ifndef CALCULATORCLIENT_H
#define CALCULATORCLIENT_H

#include "HttpClient.h"
#include "JsonHandler.h"
#include <string>
#include <memory>

class CalculatorClient {
private:
    std::string host;
    int port;
    std::unique_ptr<IHttpClient> httpClient;
    
public:
    CalculatorClient(const std::string& host = "localhost", int port = 8080,
                    std::unique_ptr<IHttpClient> client = nullptr);
    
    std::string sendCommand(const std::string& command, const std::string& user = "");
    std::string evaluateExpression(const std::string& expression, const std::string& user = "");
    
private:
    std::string createHttpRequest(const std::string& jsonBody);
};

#endif