#ifndef ICALCULATOR_H
#define ICALCULATOR_H

#include <string>

class ICalculator {
public:
    virtual ~ICalculator() = default;
    virtual double calculate(const std::string& expression, const std::string& user) = 0;
    virtual std::string evaluateExpression(const std::string& expression, const std::string& user) = 0;
};

#endif