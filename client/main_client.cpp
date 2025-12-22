#include "CalculatorClient.h"
#include <iostream>
#include <string>
#include <sstream>

void showUsage(const char* programName) {
    std::cout << "Calculator CLI Client with Sessions\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " -c <command>           Send command to server (e.g., echo, clean)\n";
    std::cout << "  " << programName << " -e <expr>              Evaluate expression\n";
    std::cout << "  " << programName << " -u <user>              Specify user for the operation\n";
    std::cout << "  " << programName << " -h                     Show this help\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " -u student -e \"pi = 3.14\"\n";
    std::cout << "  " << programName << " -u student -e \"2 * pi * 3\"\n";
    std::cout << "  " << programName << " -u student -c clean\n";
    std::cout << "  " << programName << " -e \"2 + 2\"             (uses default user)\n";
}

std::string readMultilineExpression() {
    std::string expression;
    std::string line;
    
    std::cout << "Enter expression (end with empty line):\n";
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
        if (!expression.empty()) {
            expression += "; ";
        }
        expression += line;
    }
    
    return expression;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        showUsage(argv[0]);
        return 1;
    }
    
    std::string user;
    std::string command;
    std::string expression;
    std::string option;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            showUsage(argv[0]);
            return 0;
        }
        else if (arg == "-u" && i + 1 < argc) {
            user = argv[++i];
        }
        else if (arg == "-c" && i + 1 < argc) {
            command = argv[++i];
        }
        else if (arg == "-e") {
            option = "-e";
            std::stringstream expressionStream;
            for (int j = i + 1; j < argc; j++) {
                if (j > i + 1) expressionStream << " ";
                expressionStream << argv[j];
            }
            expression = expressionStream.str();
            break;
        }
        else {
            std::cerr << "Error: Unknown argument: " << arg << std::endl;
            showUsage(argv[0]);
            return 1;
        }
    }
    
    // Если -e без аргументов, читаем из stdin
    if (option == "-e" && expression.empty()) {
        expression = readMultilineExpression();
        if (expression.empty()) {
            std::cerr << "Error: No expression provided\n";
            return 1;
        }
    }
    
    try {
        CalculatorClient client;
        std::string result;
        
        if (!command.empty()) {
            result = client.sendCommand(command, user);
        } else if (!expression.empty()) {
            result = client.evaluateExpression(expression, user);
        } else {
            std::cerr << "Error: No command or expression specified\n";
            showUsage(argv[0]);
            return 1;
        }
        
        if (!result.empty()) {
            std::cout << result << std::endl;
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}