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
    #include <sys/time.h>
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
        
        // Установка таймаута на чтение
#ifdef _WIN32
        DWORD timeout = 5000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
        
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
    static std::string createCommandJson(const std::string& command) {
        return "{\"cmd\":\"" + escapeJson(command) + "\"}";
    }
    
    static std::string createExpressionJson(const std::string& expression) {
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
        else if (jsonBody.find("\"error\":") != std::string::npos) {
            size_t start = jsonBody.find("\"error\":") + 9;
            size_t end = jsonBody.find("\"", start);
            if (end != std::string::npos) {
                return "Error: " + jsonBody.substr(start, end - start);
            }
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
    
    std::string sendCommand(const std::string& command) {
        std::string jsonBody = JsonHandler::createCommandJson(command);
        std::string httpRequest = createHttpRequest(jsonBody);
        
        HttpClient httpClient;
        std::string response = httpClient.sendRequest(host, port, httpRequest);
        
        return JsonHandler::parseResponse(response);
    }
    
    std::string evaluateExpression(const std::string& expression) {
        std::string jsonBody = JsonHandler::createExpressionJson(expression);
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
    std::cout << "Calculator CLI Client\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " -c <command>    Send command to server (e.g., echo)\n";
    std::cout << "  " << programName << " -e <expr>       Evaluate expression\n";
    std::cout << "  " << programName << " -h              Show this help\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " -c echo\n";
    std::cout << "  " << programName << " -e \"2 + 2\"\n";
    std::cout << "  " << programName << " -e \"(3 + 4) * 5\"\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        showUsage(argv[0]);
        return 1;
    }
    
    std::string option = argv[1];
    
    if (option == "-h" || option == "--help") {
        showUsage(argv[0]);
        return 0;
    }
    
    if (option == "-c" && argc >= 3) {
        std::string command = argv[2];
        
        try {
            CalculatorClient client;
            std::string result = client.sendCommand(command);
            std::cout << result << std::endl;
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    else if (option == "-e" && argc >= 3) {
        // Собираем все аргументы после -e в одно выражение
        std::stringstream expressionStream;
        for (int i = 2; i < argc; i++) {
            if (i > 2) expressionStream << " ";
            expressionStream << argv[i];
        }
        std::string expression = expressionStream.str();
        
        try {
            CalculatorClient client;
            std::string result = client.evaluateExpression(expression);
            std::cout << result << std::endl;
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "Invalid arguments!\n";
        showUsage(argv[0]);
        return 1;
    }
}