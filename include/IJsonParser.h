#ifndef IJSONPARSER_H
#define IJSONPARSER_H

#include <string>

class IJsonParser {
public:
    virtual ~IJsonParser() = default;
    virtual bool parseRequest(const std::string& json, 
                            std::string& exp, 
                            std::string& cmd, 
                            std::string& user) = 0;
    virtual std::string createResponse(const std::string& result) = 0;
    virtual std::string createResponse(double result) = 0;
    virtual std::string createEmptyResponse() = 0;
    virtual std::string createError(const std::string& error) = 0;
};

#endif