#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

// ============================================================================
// Конфигурация
// ============================================================================

#define SERVER_HOST "localhost"
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

// ============================================================================
// Сетевая часть
// ============================================================================

class HttpClient {
private:
#ifdef _WIN32
    WSADATA wsaData;
#endif

public:
    HttpClient() {
#ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
    }
    
    ~HttpClient() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    std::string sendRequest(const std::string& host, int port, const std::string& request) {
        int sock = 0;
        struct sockaddr_in serv_addr;
        
        // Создание сокета
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            throw std::runtime_error("Socket creation error");
        }
        
        // Настройка адреса сервера
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        
        // Преобразование hostname в IP
        struct hostent* server = gethostbyname(host.c_str());
        if (server == nullptr) {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            throw std::runtime_error("Cannot resolve host: " + host);
        }
        
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        
        // Подключение к серверу
        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            throw std::runtime_error("Cannot connect to server");
        }
        
        // Отправка запроса
        send(sock, request.c_str(), request.length(), 0);
        
        // Получение ответа
        char buffer[BUFFER_SIZE];
        std::string response;
        int bytes_read;
        
        // Чтение ответа
        while ((bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_read] = '\0';
            response.append(buffer);
        }
        
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        
        return response;
    }
};

// ============================================================================
// Обработка JSON
// ============================================================================

class JsonHandler {
public:
    static std::string createCommandJson(const std::string& command, const std::string& user = "") {
        if (!user.empty()) {
            return "{\"cmd\":\"" + escapeJson(command) + "\", \"user\":\"" + escapeJson(user) + "\"}";
        }
        return "{\"cmd\":\"" + escapeJson(command) + "\"}";
    }
    
    static std::string createExpressionJson(const std::string& expression, const std::string& user = "") {
        if (!user.empty()) {
            return "{\"exp\":\"" + escapeJson(expression) + "\", \"user\":\"" + escapeJson(user) + "\"}";
        }
        return "{\"exp\":\"" + escapeJson(expression) + "\"}";
    }
    
    static std::string parseResponse(const std::string& httpResponse) {
        // Ищем начало тела ответа
        size_t bodyStart = httpResponse.find("\r\n\r\n");
        if (bodyStart == std::string::npos) {
            return "Error: Invalid HTTP response";
        }
        
        std::string jsonBody = httpResponse.substr(bodyStart + 4);
        
        // Парсим простой JSON
        if (jsonBody.find("\"res\":") != std::string::npos) {
            size_t start = jsonBody.find("\"res\":") + 6;
            
            // Если результат в кавычках (строка)
            if (jsonBody[start] == '"') {
                start++;
                size_t end = jsonBody.find("\"", start);
                if (end != std::string::npos) {
                    return jsonBody.substr(start, end - start);
                }
            } 
            // Если результат число
            else {
                size_t end = jsonBody.find_first_of(",}", start);
                if (end != std::string::npos) {
                    return jsonBody.substr(start, end - start);
                }
            }
        }
        else if (jsonBody.find("\"err\":") != std::string::npos) {
            size_t start = jsonBody.find("\"err\":") + 7;
            size_t end = jsonBody.find("\"", start);
            if (end != std::string::npos) {
                return "Error: " + jsonBody.substr(start, end - start);
            }
        }
        else if (jsonBody == "{}") {
            return "";  // Пустой ответ для присваивания или команды clean
        }
        
        return "Error: Cannot parse server response";
    }
    
private:
    static std::string escapeJson(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == '"') {
                result += "\\\"";
            } else if (c == '\\') {
                result += "\\\\";
            } else if (c == '\n') {
                result += "\\n";
            } else if (c == '\r') {
                result += "\\r";
            } else if (c == '\t') {
                result += "\\t";
            } else {
                result += c;
            }
        }
        return result;
    }
};

// ============================================================================
// Калькулятор клиент
// ============================================================================

class CalculatorClient {
private:
    std::string host;
    int port;
    
public:
    CalculatorClient(const std::string& h = SERVER_HOST, int p = SERVER_PORT) 
        : host(h), port(p) {}
    
    std::string sendCommand(const std::string& command, const std::string& user = "") {
        std::string jsonBody = JsonHandler::createCommandJson(command, user);
        std::string httpRequest = createHttpRequest(jsonBody);
        
        HttpClient httpClient;
        std::string response = httpClient.sendRequest(host, port, httpRequest);
        
        return JsonHandler::parseResponse(response);
    }
    
    std::string evaluateExpression(const std::string& expression, const std::string& user = "") {
        std::string jsonBody = JsonHandler::createExpressionJson(expression, user);
        std::string httpRequest = createHttpRequest(jsonBody);
        
        HttpClient httpClient;
        std::string response = httpClient.sendRequest(host, port, httpRequest);
        
        return JsonHandler::parseResponse(response);
    }
    
private:
    std::string createHttpRequest(const std::string& jsonBody) {
        std::stringstream request;
        request << "POST / HTTP/1.1\r\n";
        request << "Host: " << host << ":" << port << "\r\n";
        request << "Content-Type: application/json\r\n";
        request << "Content-Length: " << jsonBody.length() << "\r\n";
        request << "Connection: close\r\n";
        request << "\r\n";
        request << jsonBody;
        
        return request.str();
    }
};

// ============================================================================
// Обработка аргументов командной строки
// ============================================================================

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
    
    // Проверяем, есть ли данные в stdin
    if (!std::cin.eof()) {
        // Читаем все строки
        while (std::getline(std::cin, line)) {
            if (line.empty()) {
                break;
            }
            if (!expression.empty()) {
                expression += "; ";
            }
            expression += line;
        }
    } else {
        // Нет данных в stdin, выводим инструкцию
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
    
    // Парсим аргументы
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
            // Собираем все аргументы после -e в одно выражение
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