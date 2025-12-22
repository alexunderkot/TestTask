#include "Calculator.h"
#include "ExpressionParser.h"
#include "JsonParser.h"
#include "HttpServer.h"
#include "SessionStore.h"
#include <iostream>
#include <memory>

#define DEFAULT_PORT 8080

void printServerUsage(const char* progname) {
    std::cout << "Calculator HTTP Server with Sessions\n";
    std::cout << "Usage: " << progname << " [port]\n";
    std::cout << "Default port: " << DEFAULT_PORT << std::endl;
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            printServerUsage(argv[0]);
            return 0;
        }
        
        try {
            port = std::stoi(arg);
            if (port < 1 || port > 65535) {
                std::cerr << "Port must be between 1 and 65535" << std::endl;
                return 1;
            }
        } catch (...) {
            std::cerr << "Invalid port number: " << arg << std::endl;
            return 1;
        }
    }
    
    std::cout << "Starting Calculator Server with Sessions..." << std::endl;
    
    try {
        SessionStore& sessionStore = SessionStore::instance();
        
        auto parser = std::make_unique<ExpressionParser>();
        auto calculator = std::make_unique<Calculator>(std::move(parser), sessionStore);
        auto jsonParser = std::make_unique<JsonParser>();
        
        HttpServer server(port, 
                         std::move(calculator), 
                         std::move(jsonParser),
                         sessionStore);
        
        server.start();
        
        std::cout << "Server is running. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        server.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}