#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "IJsonParser.h"
#include <string>

class JsonParser : public IJsonParser {
public:
    bool parseRequest(const std::string& json, 
                     std::string& exp, 
                     std::string& cmd, 
                     std::string& user) override;
    
    std::string createResponse(const std::string& result) override;
    std::string createResponse(double result) override;
    std::string createEmptyResponse() override;
    std::string createError(const std::string& error) override;
    
private:
    static std::string escapeJson(const std::string& str);
};

#endif