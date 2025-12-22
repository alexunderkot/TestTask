#include "JsonHandler.h"
#include <sstream>

std::string JsonHandler::createCommandJson(const std::string& command, const std::string& user) {
    if (!user.empty()) {
        return "{\"cmd\":\"" + escapeJson(command) + "\", \"user\":\"" + escapeJson(user) + "\"}";
    }
    return "{\"cmd\":\"" + escapeJson(command) + "\"}";
}

std::string JsonHandler::createExpressionJson(const std::string& expression, const std::string& user) {
    if (!user.empty()) {
        return "{\"exp\":\"" + escapeJson(expression) + "\", \"user\":\"" + escapeJson(user) + "\"}";
    }
    return "{\"exp\":\"" + escapeJson(expression) + "\"}";
}

std::string JsonHandler::parseResponse(const std::string& httpResponse) {
    size_t bodyStart = httpResponse.find("\r\n\r\n");
    if (bodyStart == std::string::npos) {
        return "Error: Invalid HTTP response";
    }
    
    std::string jsonBody = httpResponse.substr(bodyStart + 4);
    
    if (jsonBody.find("\"res\":") != std::string::npos) {
        size_t start = jsonBody.find("\"res\":") + 6;
        
        if (jsonBody[start] == '"') {
            start++;
            size_t end = jsonBody.find("\"", start);
            if (end != std::string::npos) {
                return jsonBody.substr(start, end - start);
            }
        } 
        else {
            size_t end = jsonBody.find_first_of(",}", start);
            if (end != std::string::npos) {
                return jsonBody.substr(start, end - start);
            }
        }
    }
    else if (jsonBody.find("\"err\":") != std::string::npos) {
        size_t start = jsonBody.find("\"err\":") + 7;
        size_t end = jsonBody.find("\"", start);
        if (end != std::string::npos) {
            return "Error: " + jsonBody.substr(start, end - start);
        }
    }
    else if (jsonBody == "{}") {
        return "";  // Пустой ответ для присваивания или команды clean
    }
    
    return "Error: Cannot parse server response";
}

std::string JsonHandler::escapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '"') {
            result += "\\\"";
        } else if (c == '\\') {
            result += "\\\\";
        } else if (c == '\n') {
            result += "\\n";
        } else if (c == '\r') {
            result += "\\r";
        } else if (c == '\t') {
            result += "\\t";
        } else {
            result += c;
        }
    }
    return result;
}