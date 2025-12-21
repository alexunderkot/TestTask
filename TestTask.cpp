#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

int main() {
    std::cout << "🚀 Запуск простого Echo Server на порту 8080...\n";
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "❌ Не удалось создать сокет\n";
        return 1;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "❌ Не удалось привязаться к порту 8080\n";
        return 1;
    }
    
    if (listen(server_fd, 3) < 0) {
        std::cerr << "❌ Ошибка listen\n";
        return 1;
    }
    
    std::cout << "✅ Сервер запущен! Ожидаю подключений на http://localhost:8080\n\n";
    
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            continue;
        }
        
        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer));
        
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n"
            "🔄 Echo Server работает!\n\n"
            "📨 Ваш запрос:\n" + 
            std::string(buffer) + 
            "\n📡 Отправьте что-нибудь еще!";
        
        send(client_fd, response.c_str(), response.length(), 0);
        
        close(client_fd);
    }
    
    return 0;
}