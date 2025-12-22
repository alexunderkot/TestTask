#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <string>
#include <functional>

class IExpressionParser {
public:
    virtual ~IExpressionParser() = default;
    virtual double parse(const std::string& expr, 
                        const std::function<double(const std::string&)>& variableGetter) = 0;
};

class ExpressionParser : public IExpressionParser {
public:
    double parse(const std::string& expr,
                const std::function<double(const std::string&)>& variableGetter) override;
    
private:
    double parseExpression(const std::string& expr, size_t& pos,
                          const std::function<double(const std::string&)>& variableGetter);
    double parseTerm(const std::string& expr, size_t& pos,
                    const std::function<double(const std::string&)>& variableGetter);
    double parseFactor(const std::string& expr, size_t& pos,
                      const std::function<double(const std::string&)>& variableGetter);
    double parseNumber(const std::string& expr, size_t& pos);
    std::string parseIdentifier(const std::string& expr, size_t& pos);
    std::string removeSpaces(const std::string& s);
};

#endif