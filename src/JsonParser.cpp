#include "JsonParser.h"
#include <sstream>

bool JsonParser::parseRequest(const std::string& json, 
                             std::string& exp, 
                             std::string& cmd, 
                             std::string& user) {
    size_t userPos = json.find("\"user\":\"");
    if (userPos != std::string::npos) {
        size_t start = userPos + 8;
        size_t end = json.find("\"", start);
        if (end != std::string::npos) {
            user = json.substr(start, end - start);
        }
    }
    
    size_t expPos = json.find("\"exp\":\"");
    if (expPos != std::string::npos) {
        size_t start = expPos + 7;
        size_t end = json.find("\"", start);
        if (end != std::string::npos) {
            exp = json.substr(start, end - start);
            return true;
        }
    }
    
    size_t cmdPos = json.find("\"cmd\":\"");
    if (cmdPos != std::string::npos) {
        size_t start = cmdPos + 7;
        size_t end = json.find("\"", start);
        if (end != std::string::npos) {
            cmd = json.substr(start, end - start);
            return true;
        }
    }
    
    return false;
}

std::string JsonParser::createResponse(const std::string& result) {
    return "{\"res\":\"" + escapeJson(result) + "\"}";
}

std::string JsonParser::createResponse(double result) {
    std::stringstream ss;
    ss << result;
    return "{\"res\":" + ss.str() + "}";
}

std::string JsonParser::createEmptyResponse() {
    return "{}";
}

std::string JsonParser::createError(const std::string& error) {
    return "{\"err\":\"" + escapeJson(error) + "\"}";
}

std::string JsonParser::escapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}