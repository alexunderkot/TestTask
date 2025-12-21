#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

int main() {
    std::cout << "Hello world!:)\n";
    /*
    std::cout << "=== C++ Web Server Tutorial ===" << std::endl;
    std::cout << "================================" << std::endl;
    
    // 1. Создаем HTML файл
    std::ofstream html_file("index.html");
    if (html_file.is_open()) {
        html_file << "<!DOCTYPE html>\n";
        html_file << "<html>\n";
        html_file << "<head><title>C++ Server</title></head>\n";
        html_file << "<body>\n";
        html_file << "    <h1>Hello from C++ Program 2.0!</h1>\n";
        html_file << "    <p>Compiled with Visual Studio 2022</p>\n";
        html_file << "    <p>Server ready at: http://localhost:8080</p>\n";
        html_file << "    <p>To start server, run in terminal:</p>\n";
        html_file << "    <pre>python -m http.server 8080</pre>\n";
        html_file << "    <p>Or with PowerShell:</p>\n";
        html_file << "    <pre>python -m http.server 8080</pre>\n";
        html_file << "</body>\n";
        html_file << "</html>\n";
        html_file.close();
        std::cout << "✓ Created index.html" << std::endl;
    } else {
        std::cout << "✗ Failed to create index.html" << std::endl;
    }
    
    // 2. Открываем браузер
    std::cout << "✓ Opening browser..." << std::endl;
    //system("start index.html");  // Откроет файл в браузере
    
    // 3. Инструкции
    std::cout << "\n=== INSTRUCTIONS ===" << std::endl;
    std::cout << "1. File 'index.html' has been created" << std::endl;
    std::cout << "2. To run HTTP server:" << std::endl;
    std::cout << "   - Open NEW terminal" << std::endl;
    std::cout << "   - Run: python -m http.server 8080" << std::endl;
    std::cout << "   - Or: py -m http.server 8080" << std::endl;
    std::cout << "3. Then open: http://localhost:8080" << std::endl;
    
    std::cout << "\nPress Enter to exit this program (server will continue if started)...";
    
    system("start /B python -m http.server 8080");
    system("start http://localhost:8080");
    std::string input;
    std::getline(std::cin, input);
    system("taskkill /F /IM python.exe 2>nul");
    
    std::cout << "\nProgram finished. Bye!" << std::endl;
    */
    return 0;
}