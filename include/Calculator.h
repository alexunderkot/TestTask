
#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "ICalculator.h"
#include "ExpressionParser.h"
#include "ISessionStore.h"
#include <string>
#include <vector>
#include <sstream>
#include <memory>

class Calculator : public ICalculator {
private:
    std::unique_ptr<IExpressionParser> parser;
    ISessionStore& sessionStore;
    
public:
    Calculator(std::unique_ptr<IExpressionParser> parser, ISessionStore& store);
    
    double calculate(const std::string& expression, const std::string& user) override;
    std::string evaluateExpression(const std::string& expression, const std::string& user) override;
    
private:
    bool isAssignment(const std::string& line, std::string& varName, std::string& expr);
    bool isValidVariableName(const std::string& name);
};

#endif