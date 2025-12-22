#include "Calculator.h"
#include <stdexcept>
#include <algorithm>
#include <cctype>

Calculator::Calculator(std::unique_ptr<IExpressionParser> parser, ISessionStore& store)
    : parser(std::move(parser)), sessionStore(store) {}

double Calculator::calculate(const std::string& expression, const std::string& user) {
    std::string effectiveUser = sessionStore.getEffectiveUser(user);
    
    return parser->parse(expression, 
        [this, &effectiveUser](const std::string& varName) {
            return sessionStore.get(effectiveUser, varName);
        });
}

std::string Calculator::evaluateExpression(const std::string& expression, const std::string& user) {
    std::stringstream result;
    std::stringstream exprStream(expression);
    std::string line;
    double lastResult = 0;
    std::string effectiveUser = sessionStore.getEffectiveUser(user);
    
    while (std::getline(exprStream, line, ';')) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (line.empty()) continue;
        
        std::string varName, expr;
        if (isAssignment(line, varName, expr)) {
            if (!isValidVariableName(varName)) {
                throw std::runtime_error("Invalid variable name: " + varName);
            }
            
            double value = calculate(expr, effectiveUser);
            sessionStore.set(effectiveUser, varName, value);
            lastResult = value;
        } else {
            lastResult = calculate(line, effectiveUser);
            if (result.tellp() > 0) {
                result << "; ";
            }
            result << lastResult;
        }
    }
    
    return result.str();
}

bool Calculator::isAssignment(const std::string& line, std::string& varName, std::string& expr) {
    size_t assignPos = line.find('=');
    if (assignPos == std::string::npos) {
        return false;
    }
    
    varName = line.substr(0, assignPos);
    expr = line.substr(assignPos + 1);
    
    varName.erase(std::remove_if(varName.begin(), varName.end(), 
                               [](unsigned char c){ return std::isspace(c); }), 
                varName.end());
    
    return !varName.empty();
}

bool Calculator::isValidVariableName(const std::string& name) {
    if (name.empty()) return false;
    
    for (char c : name) {
        if (!std::isalpha(c) && c != '_') {
            return false;
        }
    }
    return true;
}