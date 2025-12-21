#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <cmath>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <mutex>
#include <algorithm>
#include <unordered_map>

// ============================================================================
// Библиотеки для работы с сетью
// ============================================================================

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 4096

// ============================================================================
// Глобальное состояние (сессии пользователей)
// ============================================================================

class SessionStore {
private:
    // user_id -> variables
    std::unordered_map<std::string, std::map<std::string, double>> sessions;
    std::mutex mutex;
    
    static const std::string DEFAULT_USER;
    
public:
    static SessionStore& instance() {
        static SessionStore instance;
        return instance;
    }
    
    void set(const std::string& user, const std::string& name, double value) {
        std::lock_guard<std::mutex> lock(mutex);
        sessions[user][name] = value;
    }
    
    double get(const std::string& user, const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex);
        auto userIt = sessions.find(user);
        if (userIt == sessions.end()) {
            throw std::runtime_error("Unknown variable '" + name + "'");
        }
        
        auto varIt = userIt->second.find(name);
        if (varIt == userIt->second.end()) {
            throw std::runtime_error("Unknown variable '" + name + "'");
        }
        return varIt->second;
    }
    
    bool exists(const std::string& user, const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex);
        auto userIt = sessions.find(user);
        if (userIt == sessions.end()) {
            return false;
        }
        return userIt->second.find(name) != userIt->second.end();
    }
    
    void clear(const std::string& user) {
        std::lock_guard<std::mutex> lock(mutex);
        sessions[user].clear();
    }
    
    void clearAll() {
        std::lock_guard<std::mutex> lock(mutex);
        sessions.clear();
    }
    
    void list(const std::string& user) {
        std::lock_guard<std::mutex> lock(mutex);
        auto userIt = sessions.find(user);
        if (userIt != sessions.end()) {
            for (const auto& pair : userIt->second) {
                std::cout << pair.first << " = " << pair.second << std::endl;
            }
        }
    }
    
    std::string getEffectiveUser(const std::string& requestedUser) {
        if (requestedUser.empty()) {
            return DEFAULT_USER;
        }
        return requestedUser;
    }
};

const std::string SessionStore::DEFAULT_USER = "default";

// ============================================================================
// Простой JSON парсер
// ============================================================================

class JsonParser {
public:
    static std::string createResponse(const std::string& result) {
        return "{\"res\":\"" + escapeJson(result) + "\"}";
    }
    
    static std::string createResponse(double result) {
        std::stringstream ss;
        ss << result;
        return "{\"res\":" + ss.str() + "}";
    }
    
    static std::string createEmptyResponse() {
        return "{}";
    }
    
    static std::string createError(const std::string& error) {
        return "{\"err\":\"" + escapeJson(error) + "\"}";
    }
    
    static bool parseRequest(const std::string& json, 
                            std::string& exp, 
                            std::string& cmd, 
                            std::string& user) {
        // Ищем user
        size_t userPos = json.find("\"user\":\"");
        if (userPos != std::string::npos) {
            size_t start = userPos + 8;
            size_t end = json.find("\"", start);
            if (end != std::string::npos) {
                user = json.substr(start, end - start);
            }
        }
        
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
    
private:
    static std::string escapeJson(const std::string& str) {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }
};

// ============================================================================
// Калькулятор с поддержкой сессий
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
    
    static double parseExpression(const std::string& expr, size_t& pos, const std::string& user);
    static double parseTerm(const std::string& expr, size_t& pos, const std::string& user);
    static double parseFactor(const std::string& expr, size_t& pos, const std::string& user);
    static double parseNumber(const std::string& expr, size_t& pos);
    static std::string parseIdentifier(const std::string& expr, size_t& pos);
    
public:
    static std::string evaluateExpression(const std::string& expression, const std::string& user) {
        std::stringstream result;
        std::stringstream exprStream(expression);
        std::string line;
        double lastResult = 0;
        std::string effectiveUser = SessionStore::instance().getEffectiveUser(user);
        
        while (std::getline(exprStream, line, ';')) {
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
            
            if (line.empty()) continue;
            
            try {
                // Проверяем, является ли это присваиванием
                size_t assignPos = line.find('=');
                if (assignPos != std::string::npos) {
                    // Это присваивание
                    std::string varName = line.substr(0, assignPos);
                    std::string expr = line.substr(assignPos + 1);
                    
                    // Удаляем пробелы из имени переменной
                    varName.erase(std::remove_if(varName.begin(), varName.end(), 
                                               [](unsigned char c){ return std::isspace(c); }), 
                                varName.end());
                    
                    if (varName.empty()) {
                        throw std::runtime_error("Invalid variable name");
                    }
                    
                    // Проверяем, что имя переменной состоит только из букв и подчеркиваний
                    for (char c : varName) {
                        if (!std::isalpha(c) && c != '_') {
                            throw std::runtime_error("Invalid variable name: " + varName);
                        }
                    }
                    
                    // Вычисляем значение
                    double value = calculate(expr, effectiveUser);
                    SessionStore::instance().set(effectiveUser, varName, value);
                    lastResult = value;
                } else {
                    // Это обычное выражение
                    lastResult = calculate(line, effectiveUser);
                    if (result.tellp() > 0) {
                        result << "; ";
                    }
                    result << lastResult;
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(e.what());
            }
        }
        
        return result.str();
    }
    
    static double calculate(const std::string& expression, const std::string& user) {
        std::string expr = removeSpaces(expression);
        size_t pos = 0;
        double result = parseExpression(expr, pos, user);
        
        if (pos < expr.length()) {
            throw std::runtime_error("Unexpected characters: " + expr.substr(pos));
        }
        
        return result;
    }
};

double Calculator::parseExpression(const std::string& expr, size_t& pos, const std::string& user) {
    double result = parseTerm(expr, pos, user);
    
    while (pos < expr.length() && (expr[pos] == '+' || expr[pos] == '-')) {
        char op = expr[pos];
        pos++;
        
        double right = parseTerm(expr, pos, user);
        
        if (op == '+') {
            result += right;
        } else { // op == '-'
            result -= right;
        }
    }
    
    return result;
}

double Calculator::parseTerm(const std::string& expr, size_t& pos, const std::string& user) {
    double result = parseFactor(expr, pos, user);
    
    while (pos < expr.length() && (expr[pos] == '*' || expr[pos] == '/')) {
        char op = expr[pos];
        pos++;
        
        double right = parseFactor(expr, pos, user);
        
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

double Calculator::parseFactor(const std::string& expr, size_t& pos, const std::string& user) {
    if (pos >= expr.length()) {
        throw std::runtime_error("Unexpected end of expression");
    }
    
    if (expr[pos] == '(') {
        pos++; // skip '('
        double result = parseExpression(expr, pos, user);
        
        if (pos >= expr.length() || expr[pos] != ')') {
            throw std::runtime_error("Missing ')'");
        }
        pos++; // skip ')'
        return result;
    }
    
    // Check for unary minus
    if (expr[pos] == '-') {
        pos++;
        return -parseFactor(expr, pos, user);
    }
    
    // Check for variable
    if (std::isalpha(expr[pos]) || expr[pos] == '_') {
        std::string varName = parseIdentifier(expr, pos);
        return SessionStore::instance().get(user, varName);
    }
    
    return parseNumber(expr, pos);
}

double Calculator::parseNumber(const std::string& expr, size_t& pos) {
    if (pos >= expr.length()) {
        throw std::runtime_error("Expected number");
    }
    
    size_t start = pos;
    
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

std::string Calculator::parseIdentifier(const std::string& expr, size_t& pos) {
    size_t start = pos;
    
    while (pos < expr.length() && (std::isalnum(expr[pos]) || expr[pos] == '_')) {
        pos++;
    }
    
    if (start == pos) {
        throw std::runtime_error("Invalid identifier");
    }
    
    return expr.substr(start, pos - start);
}

// ============================================================================
// HTTP сервер
// ============================================================================

class HttpServer {
private:
    int server_fd;
    bool running;
    int port;
    
public:
    HttpServer(int p = DEFAULT_PORT) : running(false), port(p) {}
    
    ~HttpServer() {
        stop();
    }
    
    void start() {
        if (running) return;
        
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
        
        std::cout << "Calculator Server with Sessions running on port " << port << std::endl;
        std::cout << "Use Ctrl+C to stop" << std::endl;
        
        running = true;
        run();
    }
    
    void stop() {
        if (!running) return;
        running = false;
        close(server_fd);
    }
    
    bool isRunning() const { return running; }
    
private:
    void run() {
        while (running) {
            int new_socket;
            struct sockaddr_in address;
            socklen_t addrlen = sizeof(address);
            
            // Принятие соединения
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                if (running) perror("accept");
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
    }
    
    std::string processHttpRequest(const std::string& request) {
        // Ищем тело запроса
        size_t body_start = request.find("\r\n\r\n");
        if (body_start == std::string::npos) {
            return createHttpResponse(JsonParser::createError("Invalid HTTP request"));
        }
        
        std::string body = request.substr(body_start + 4);
        
        // Парсим JSON
        std::string exp, cmd, user;
        if (!JsonParser::parseRequest(body, exp, cmd, user)) {
            return createHttpResponse(JsonParser::createError("Invalid JSON"));
        }
        
        // Обрабатываем команду
        if (!cmd.empty()) {
            if (cmd == "echo") {
                return createHttpResponse(JsonParser::createResponse("echo"));
            } else if (cmd == "clean") {
                std::string effectiveUser = SessionStore::instance().getEffectiveUser(user);
                SessionStore::instance().clear(effectiveUser);
                return createHttpResponse(JsonParser::createEmptyResponse());
            } else {
                return createHttpResponse(JsonParser::createError("Unknown command"));
            }
        }
        
        // Вычисляем выражение
        if (!exp.empty()) {
            try {
                std::string result = Calculator::evaluateExpression(exp, user);
                if (result.empty()) {
                    return createHttpResponse(JsonParser::createEmptyResponse());
                } else {
                    try {
                        // Если результат содержит только число
                        if (result.find(';') == std::string::npos) {
                            double numResult = std::stod(result);
                            return createHttpResponse(JsonParser::createResponse(numResult));
                        } else {
                            // Если несколько результатов
                            return createHttpResponse(JsonParser::createResponse(result));
                        }
                    } catch (...) {
                        // Если не число, возвращаем как строку
                        return createHttpResponse(JsonParser::createResponse(result));
                    }
                }
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
// Главная функция сервера
// ============================================================================

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
        HttpServer server(port);
        
        server.start();
        
        // Простой способ удержать программу запущенной
        std::cout << "Server is running. Press Enter to stop..." << std::endl;
        std::cin.get();
        
        server.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}