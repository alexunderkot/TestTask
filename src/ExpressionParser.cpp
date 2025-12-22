#include "ExpressionParser.h"
#include <stdexcept>
#include <cctype>
#include <algorithm>

std::string ExpressionParser::removeSpaces(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (!std::isspace(c)) {
            result += c;
        }
    }
    return result;
}

double ExpressionParser::parse(const std::string& expr,
                             const std::function<double(const std::string&)>& variableGetter) {
    std::string cleanedExpr = removeSpaces(expr);
    size_t pos = 0;
    double result = parseExpression(cleanedExpr, pos, variableGetter);
    
    if (pos < cleanedExpr.length()) {
        throw std::runtime_error("Unexpected characters: " + cleanedExpr.substr(pos));
    }
    
    return result;
}

double ExpressionParser::parseExpression(const std::string& expr, size_t& pos,
                                       const std::function<double(const std::string&)>& variableGetter) {
    double result = parseTerm(expr, pos, variableGetter);
    
    while (pos < expr.length() && (expr[pos] == '+' || expr[pos] == '-')) {
        char op = expr[pos];
        pos++;
        
        double right = parseTerm(expr, pos, variableGetter);
        
        if (op == '+') {
            result += right;
        } else { // op == '-'
            result -= right;
        }
    }
    
    return result;
}

double ExpressionParser::parseTerm(const std::string& expr, size_t& pos,
                                 const std::function<double(const std::string&)>& variableGetter) {
    double result = parseFactor(expr, pos, variableGetter);
    
    while (pos < expr.length() && (expr[pos] == '*' || expr[pos] == '/')) {
        char op = expr[pos];
        pos++;
        
        double right = parseFactor(expr, pos, variableGetter);
        
        if (op == '*') {
            result *= right;
        } else { // op == '/'
            if (right == 0) {
                throw std::runtime_error("Division by zero");
            }
            result /= right;
        }
    }
    
    return result;
}

double ExpressionParser::parseFactor(const std::string& expr, size_t& pos,
                                   const std::function<double(const std::string&)>& variableGetter) {
    if (pos >= expr.length()) {
        throw std::runtime_error("Unexpected end of expression");
    }
    
    if (expr[pos] == '(') {
        pos++; // skip '('
        double result = parseExpression(expr, pos, variableGetter);
        
        if (pos >= expr.length() || expr[pos] != ')') {
            throw std::runtime_error("Missing ')'");
        }
        pos++; // skip ')'
        return result;
    }
    
    if (expr[pos] == '-') {
        pos++;
        return -parseFactor(expr, pos, variableGetter);
    }
    
    if (std::isalpha(expr[pos]) || expr[pos] == '_') {
        std::string varName = parseIdentifier(expr, pos);
        return variableGetter(varName);
    }
    
    return parseNumber(expr, pos);
}

double ExpressionParser::parseNumber(const std::string& expr, size_t& pos) {
    if (pos >= expr.length()) {
        throw std::runtime_error("Expected number");
    }
    
    size_t start = pos;
    
    while (pos < expr.length() && std::isdigit(expr[pos])) {
        pos++;
    }
    
    if (pos < expr.length() && expr[pos] == '.') {
        pos++;
        while (pos < expr.length() && std::isdigit(expr[pos])) {
            pos++;
        }
    }
    
    if (start == pos) {
        throw std::runtime_error("Invalid number");
    }
    
    std::string numStr = expr.substr(start, pos - start);
    return std::stod(numStr);
}

std::string ExpressionParser::parseIdentifier(const std::string& expr, size_t& pos) {
    size_t start = pos;
    
    while (pos < expr.length() && (std::isalnum(expr[pos]) || expr[pos] == '_')) {
        pos++;
    }
    
    if (start == pos) {
        throw std::runtime_error("Invalid identifier");
    }
    
    return expr.substr(start, pos - start);
}