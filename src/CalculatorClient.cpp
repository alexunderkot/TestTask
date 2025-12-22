#include "CalculatorClient.h"
#include <sstream>

CalculatorClient::CalculatorClient(const std::string& host, int port,
                                 std::unique_ptr<IHttpClient> client)
    : host(host), port(port), 
      httpClient(client ? std::move(client) : std::make_unique<HttpClient>()) {}

std::string CalculatorClient::sendCommand(const std::string& command, const std::string& user) {
    std::string jsonBody = JsonHandler::createCommandJson(command, user);
    std::string httpRequest = createHttpRequest(jsonBody);
    
    std::string response = httpClient->sendRequest(host, port, httpRequest);
    
    return JsonHandler::parseResponse(response);
}

std::string CalculatorClient::evaluateExpression(const std::string& expression, const std::string& user) {
    std::string jsonBody = JsonHandler::createExpressionJson(expression, user);
    std::string httpRequest = createHttpRequest(jsonBody);
    
    std::string response = httpClient->sendRequest(host, port, httpRequest);
    
    return JsonHandler::parseResponse(response);
}

std::string CalculatorClient::createHttpRequest(const std::string& jsonBody) {
    std::stringstream request;
    request << "POST / HTTP/1.1\r\n";
    request << "Host: " << host << ":" << port << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << jsonBody.length() << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    request << jsonBody;
    
    return request.str();
}