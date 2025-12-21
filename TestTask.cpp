#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <cmath>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>

// ============================================================================
// Библиотеки для работы с сетью (для Linux)
// ============================================================================

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// ============================================================================
// Простой JSON парсер
// ============================================================================

class JsonParser {
public:
    static std::string createResponse(const std::string& result) {
        return "{\"res\":\"" + result + "\"}";
    }
    
    static std::string createResponse(double result) {
        std::stringstream ss;
        ss << result;
        return "{\"res\":" + ss.str() + "}";
    }
    
    static std::string createError(const std::string& error) {
        return "{\"error\":\"" + error + "\"}";
    }
    
    static bool parseRequest(const std::string& json, std::string& exp, std::string& cmd) {
        // Ищем exp
        size_t expPos = json.find("\"exp\":\"");
        if (expPos != std::string::npos) {
            size_t start = expPos + 7;
            size_t end = json.find("\"", start);
            if (end != std::string::npos) {
                exp = json.substr(start, end - start);
                return true;
            }
        }
        
        // Ищем cmd
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
};

// ============================================================================
// Правильный калькулятор с поддержкой всех операций
// ============================================================================

class Calculator {
private:
    static std::string removeSpaces(const std::string& s) {
        std::string result;
        for (char c : s) {
            if (!std::isspace(c)) {
                result += c;
            }
        }
        return result;
    }
    
    static double parseExpression(const std::string& expr, size_t& pos);
    static double parseTerm(const std::string& expr, size_t& pos);
    static double parseFactor(const std::string& expr, size_t& pos);
    static double parseNumber(const std::string& expr, size_t& pos);
    
public:
    static double calculate(const std::string& expression) {
        std::string expr = removeSpaces(expression);
        size_t pos = 0;
        double result = parseExpression(expr, pos);
        
        if (pos < expr.length()) {
            throw std::runtime_error("Unexpected characters: " + expr.substr(pos));
        }
        
        return result;
    }
};

double Calculator::parseExpression(const std::string& expr, size_t& pos) {
    double result = parseTerm(expr, pos);
    
    while (pos < expr.length() && (expr[pos] == '+' || expr[pos] == '-')) {
        char op = expr[pos];
        pos++;
        
        double right = parseTerm(expr, pos);
        
        if (op == '+') {
            result += right;
        } else { // op == '-'
            result -= right;
        }
    }
    
    return result;
}

double Calculator::parseTerm(const std::string& expr, size_t& pos) {
    double result = parseFactor(expr, pos);
    
    while (pos < expr.length() && (expr[pos] == '*' || expr[pos] == '/')) {
        char op = expr[pos];
        pos++;
        
        double right = parseFactor(expr, pos);
        
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

double Calculator::parseFactor(const std::string& expr, size_t& pos) {
    if (pos >= expr.length()) {
        throw std::runtime_error("Unexpected end of expression");
    }
    
    if (expr[pos] == '(') {
        pos++; // skip '('
        double result = parseExpression(expr, pos);
        
        if (pos >= expr.length() || expr[pos] != ')') {
            throw std::runtime_error("Missing ')'");
        }
        pos++; // skip ')'
        return result;
    }
    
    // Check for unary minus
    if (expr[pos] == '-') {
        pos++;
        return -parseFactor(expr, pos);
    }
    
    return parseNumber(expr, pos);
}

double Calculator::parseNumber(const std::string& expr, size_t& pos) {
    if (pos >= expr.length()) {
        throw std::runtime_error("Expected number");
    }
    
    size_t start = pos;
    
    // Check for leading minus (already handled in parseFactor)
    
    // Read digits
    while (pos < expr.length() && std::isdigit(expr[pos])) {
        pos++;
    }
    
    // Decimal point
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

// ============================================================================
// HTTP сервер
// ============================================================================

class SimpleHttpServer {
private:
    int server_fd;
    bool running;
    
public:
    SimpleHttpServer() : running(false) {}
    
    void start(int port = PORT) {
        // Создание сокета
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        
        // Настройка сокета
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        
        // Настройка адреса
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        // Привязка сокета
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        
        // Прослушивание
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Calculator Server running on port " << port << std::endl;
        std::cout << "Use Ctrl+C to stop" << std::endl;
        
        running = true;
        run();
    }
    
private:
    void run() {
        while (running) {
            int new_socket;
            struct sockaddr_in address;
            int addrlen = sizeof(address);
            
            // Принятие соединения
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                                    (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                continue;
            }
            
            // Обработка клиента в отдельном потоке
            std::thread([this, new_socket, address]() {
                handleClient(new_socket, address);
            }).detach();
        }
    }
    
    void handleClient(int socket, struct sockaddr_in address) {
        char buffer[BUFFER_SIZE] = {0};
        std::string client_ip = inet_ntoa(address.sin_addr);
        int client_port = ntohs(address.sin_port);
        
        std::cout << "New connection from " << client_ip << ":" << client_port << std::endl;
        
        // Чтение запроса
        ssize_t bytes_read = read(socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            std::string request(buffer, bytes_read);
            
            // Обработка запроса
            std::string response = processHttpRequest(request);
            
            // Отправка ответа
            send(socket, response.c_str(), response.length(), 0);
        }
        
        close(socket);
        std::cout << "Connection closed: " << client_ip << ":" << client_port << std::endl;
    }
    
    std::string processHttpRequest(const std::string& request) {
        // Ищем тело запроса
        size_t body_start = request.find("\r\n\r\n");
        if (body_start == std::string::npos) {
            return createHttpResponse(JsonParser::createError("Invalid HTTP request"));
        }
        
        std::string body = request.substr(body_start + 4);
        
        // Парсим JSON
        std::string exp, cmd;
        if (!JsonParser::parseRequest(body, exp, cmd)) {
            return createHttpResponse(JsonParser::createError("Invalid JSON"));
        }
        
        // Обрабатываем команду
        if (!cmd.empty()) {
            if (cmd == "echo") {
                return createHttpResponse(JsonParser::createResponse("echo"));
            } else {
                return createHttpResponse(JsonParser::createError("Unknown command"));
            }
        }
        
        // Вычисляем выражение
        if (!exp.empty()) {
            try {
                double result = Calculator::calculate(exp);
                return createHttpResponse(JsonParser::createResponse(result));
            } catch (const std::exception& e) {
                return createHttpResponse(JsonParser::createError(e.what()));
            }
        }
        
        return createHttpResponse(JsonParser::createError("No expression provided"));
    }
    
    std::string createHttpResponse(const std::string& json_body) {
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(json_body.length()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            json_body;
        
        return response;
    }
};

// ============================================================================
// Главная функция
// ============================================================================

int main(int argc, char* argv[]) {
    int port = PORT;
    
    if (argc > 1) {
        if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            std::cout << "Calculator HTTP Server\n";
            std::cout << "Usage: " << argv[0] << " [port]\n";
            std::cout << "Default port: " << PORT << std::endl;
            return 0;
        }
        
        try {
            port = std::stoi(argv[1]);
            if (port < 1 || port > 65535) {
                std::cerr << "Port must be between 1 and 65535" << std::endl;
                return 1;
            }
        } catch (...) {
            std::cerr << "Invalid port number" << std::endl;
            return 1;
        }
    }
    
    std::cout << "Starting Calculator Server..." << std::endl;
    
    try {
        SimpleHttpServer server;
        server.start(port);
        
        // Держим программу запущенной
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}