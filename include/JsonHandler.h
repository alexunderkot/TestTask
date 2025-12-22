#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <string>

class JsonHandler {
public:
    static std::string createCommandJson(const std::string& command, const std::string& user = "");
    static std::string createExpressionJson(const std::string& expression, const std::string& user = "");
    static std::string parseResponse(const std::string& httpResponse);
    
private:
    static std::string escapeJson(const std::string& str);
};

#endif