#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <cmath>
#include <iomanip>

// ============================================================================
// Классы для работы с токенами
// ============================================================================

enum class TokenType {
    NUMBER,
    OPERATOR,
    PAREN_LEFT,
    PAREN_RIGHT,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    
    Token(TokenType t, const std::string& v) : type(t), value(v) {}
};

// ============================================================================
// Класс для лексического анализа (токенизации)
// ============================================================================

class Tokenizer {
public:
    static std::vector<Token> tokenize(const std::string& expression) {
        std::vector<Token> tokens;
        std::string numberBuffer;
        
        for (size_t i = 0; i < expression.length(); i++) {
            char c = expression[i];
            
            // Пропускаем пробелы
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!numberBuffer.empty()) {
                    tokens.emplace_back(TokenType::NUMBER, numberBuffer);
                    numberBuffer.clear();
                }
                continue;
            }
            
            // Собираем числа (включая десятичную точку)
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
                numberBuffer += c;
                continue;
            }
            
            // Если собрали число, добавляем его как токен
            if (!numberBuffer.empty()) {
                tokens.emplace_back(TokenType::NUMBER, numberBuffer);
                numberBuffer.clear();
            }
            
            // Операторы и скобки
            if (isOperator(c)) {
                tokens.emplace_back(TokenType::OPERATOR, std::string(1, c));
            } else if (c == '(') {
                tokens.emplace_back(TokenType::PAREN_LEFT, "(");
            } else if (c == ')') {
                tokens.emplace_back(TokenType::PAREN_RIGHT, ")");
            } else {
                throw std::runtime_error("Неверный символ: " + std::string(1, c));
            }
        }
        
        // Добавляем последнее число, если есть
        if (!numberBuffer.empty()) {
            tokens.emplace_back(TokenType::NUMBER, numberBuffer);
        }
        
        return tokens;
    }
    
private:
    static bool isOperator(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/';
    }
};

// ============================================================================
// Класс для преобразования в прямую польскую нотацию (префиксную)
// ============================================================================

class PolishNotationConverter {
public:
    // Преобразование инфиксной записи в префиксную (прямую польскую)
    static std::vector<Token> toPrefix(const std::string& expression) {
        std::vector<Token> tokens = Tokenizer::tokenize(expression);
        return infixToPrefix(tokens);
    }
    
    // Вычисление выражения в префиксной нотации
    static double evaluatePrefix(const std::vector<Token>& prefixTokens) {
        std::stack<double> values;
        
        // Обрабатываем токены справа налево для префиксной записи
        for (auto it = prefixTokens.rbegin(); it != prefixTokens.rend(); ++it) {
            const Token& token = *it;
            
            if (token.type == TokenType::NUMBER) {
                values.push(std::stod(token.value));
            } else if (token.type == TokenType::OPERATOR) {
                if (values.size() < 2) {
                    throw std::runtime_error("Недостаточно операндов для оператора " + token.value);
                }
                
                double a = values.top();
                values.pop();
                double b = values.top();
                values.pop();
                
                double result = applyOperator(token.value, a, b);
                values.push(result);
            } else {
                throw std::runtime_error("Неверный тип токена в префиксном выражении");
            }
        }
        
        if (values.size() != 1) {
            throw std::runtime_error("Некорректное выражение");
        }
        
        return values.top();
    }
    
    // Форматирование префиксного выражения в строку
    static std::string formatPrefix(const std::vector<Token>& prefixTokens) {
        std::stringstream ss;
        for (size_t i = 0; i < prefixTokens.size(); i++) {
            ss << prefixTokens[i].value;
            if (i < prefixTokens.size() - 1) {
                ss << " ";
            }
        }
        return ss.str();
    }
    
private:
    // Преобразование инфиксной записи в префиксную
    static std::vector<Token> infixToPrefix(const std::vector<Token>& infixTokens) {
        std::stack<Token> operators;
        std::stack<std::vector<Token>> operands;
        
        for (const auto& token : infixTokens) {
            if (token.type == TokenType::NUMBER) {
                std::vector<Token> single;
                single.push_back(token);
                operands.push(single);
            } else if (token.type == TokenType::PAREN_LEFT) {
                operators.push(token);
            } else if (token.type == TokenType::PAREN_RIGHT) {
                while (!operators.empty() && operators.top().type != TokenType::PAREN_LEFT) {
                    applyOperatorToOperands(operators, operands);
                }
                
                if (operators.empty()) {
                    throw std::runtime_error("Несогласованные скобки");
                }
                operators.pop(); // Удаляем '('
            } else if (token.type == TokenType::OPERATOR) {
                while (!operators.empty() && 
                       operators.top().type != TokenType::PAREN_LEFT &&
                       getPrecedence(operators.top().value) >= getPrecedence(token.value)) {
                    applyOperatorToOperands(operators, operands);
                }
                operators.push(token);
            }
        }
        
        while (!operators.empty()) {
            if (operators.top().type == TokenType::PAREN_LEFT) {
                throw std::runtime_error("Несогласованные скобки");
            }
            applyOperatorToOperands(operators, operands);
        }
        
        if (operands.size() != 1) {
            throw std::runtime_error("Некорректное выражение");
        }
        
        return operands.top();
    }
    
    // Применение оператора к операндам в стеке
    static void applyOperatorToOperands(std::stack<Token>& operators, 
                                        std::stack<std::vector<Token>>& operands) {
        if (operands.size() < 2) {
            throw std::runtime_error("Недостаточно операндов");
        }
        
        Token op = operators.top();
        operators.pop();
        
        std::vector<Token> b = operands.top();
        operands.pop();
        std::vector<Token> a = operands.top();
        operands.pop();
        
        // Создаем префиксное выражение: оператор + операнд1 + операнд2
        std::vector<Token> newOperand;
        newOperand.push_back(op);
        newOperand.insert(newOperand.end(), a.begin(), a.end());
        newOperand.insert(newOperand.end(), b.begin(), b.end());
        
        operands.push(newOperand);
    }
    
    // Получение приоритета оператора
    static int getPrecedence(const std::string& op) {
        if (op == "+" || op == "-") {
            return 1;
        } else if (op == "*" || op == "/") {
            return 2;
        }
        return 0;
    }
    
    // Применение оператора к двум числам
    static double applyOperator(const std::string& op, double a, double b) {
        if (op == "+") {
            return a + b;
        } else if (op == "-") {
            return a - b;
        } else if (op == "*") {
            return a * b;
        } else if (op == "/") {
            if (std::fabs(b) < 1e-12) {
                throw std::runtime_error("Деление на ноль");
            }
            return a / b;
        }
        throw std::runtime_error("Неизвестный оператор: " + op);
    }
};

// ============================================================================
// Класс калькулятора (основной интерфейс)
// ============================================================================

class Calculator {
public:
    // Вычисление выражения
    static double calculate(const std::string& expression) {
        std::vector<Token> prefixTokens = PolishNotationConverter::toPrefix(expression);
        return PolishNotationConverter::evaluatePrefix(prefixTokens);
    }
    
    // Получение выражения в прямой польской нотации
    static std::string getPrefixNotation(const std::string& expression) {
        std::vector<Token> prefixTokens = PolishNotationConverter::toPrefix(expression);
        return PolishNotationConverter::formatPrefix(prefixTokens);
    }
    
    // Интерактивный режим работы
    static void runInteractive() {
        std::cout << "=============================================\n";
        std::cout << "  КАЛЬКУЛЯТОР С ПРЯМОЙ ПОЛЬСКОЙ НОТАЦИЕЙ\n";
        std::cout << "=============================================\n";
        std::cout << "Поддерживаемые операции: + - * / ( )\n";
        std::cout << "Примеры выражений:\n";
        std::cout << "  (3 + 4) * 5\n";
        std::cout << "  10 + 2 * 3\n";
        std::cout << "  3 + 4 * 2 / (1 - 5)\n";
        std::cout << "Введите 'exit' для выхода\n";
        std::cout << "=============================================\n\n";
        
        std::string input;
        
        while (true) {
            std::cout << "Введите выражение> ";
            std::getline(std::cin, input);
            
            // Проверка на выход
            if (input == "exit" || input == "quit" || input == "q") {
                std::cout << "Выход из программы...\n";
                break;
            }
            
            if (input.empty()) {
                continue;
            }
            
            try {
                // Вычисляем результат
                double result = calculate(input);
                
                // Получаем префиксную запись
                std::string prefixNotation = getPrefixNotation(input);
                
                // Выводим результаты
                std::cout << "┌─────────────────────────────────────┐\n";
                std::cout << "│ Исходное выражение: " << std::setw(15) << input << " │\n";
                std::cout << "│ Польская запись:    " << std::setw(15) << prefixNotation << " │\n";
                std::cout << "│ Результат:          " << std::setw(15) << std::fixed 
                         << std::setprecision(6) << result << " │\n";
                std::cout << "└─────────────────────────────────────┘\n\n";
                
            } catch (const std::exception& e) {
                std::cerr << "ОШИБКА: " << e.what() << "\n\n";
            }
        }
    }
    
    // Тестовый режим
    static void runTests() {
        std::cout << "Запуск тестов...\n\n";
        
        struct TestCase {
            std::string expression;
            double expected;
            std::string description;
        };
        
        std::vector<TestCase> tests = {
            {"1 + 2", 3, "Простое сложение"},
            {"5 - 3", 2, "Простое вычитание"},
            {"4 * 3", 12, "Простое умножение"},
            {"10 / 2", 5, "Простое деление"},
            {"2 + 3 * 4", 14, "Приоритет умножения"},
            {"(2 + 3) * 4", 20, "Скобки меняют приоритет"},
            {"10 - 3 - 2", 5, "Левая ассоциативность"},
            {"3.5 + 2.5", 6.0, "Вещественные числа"},
            {"(1 + 2) * (3 + 4)", 21, "Вложенные скобки"},
            {"3 + 4 * 2 / (1 - 5)", 1, "Сложное выражение"},
            {"0.1 + 0.2", 0.3, "Вещественное сложение"}
        };
        
        int passed = 0;
        int total = tests.size();
        
        for (const auto& test : tests) {
            try {
                double result = calculate(test.expression);
                double tolerance = 1e-10;
                
                if (std::fabs(result - test.expected) < tolerance) {
                    std::cout << "✓ PASS: " << test.description << " (" 
                             << test.expression << " = " << result << ")\n";
                    passed++;
                } else {
                    std::cout << "✗ FAIL: " << test.description 
                             << " (ожидалось: " << test.expected 
                             << ", получено: " << result << ")\n";
                }
                
                // Дополнительно покажем польскую запись
                std::string prefix = getPrefixNotation(test.expression);
                std::cout << "  Польская запись: " << prefix << "\n";
                
            } catch (const std::exception& e) {
                std::cout << "✗ ERROR: " << test.description 
                         << " - " << e.what() << "\n";
            }
            std::cout << "\n";
        }
        
        std::cout << "=============================================\n";
        std::cout << "Результат тестов: " << passed << "/" << total << " пройдено\n";
        std::cout << "=============================================\n\n";
    }
};

// ============================================================================
// Главная функция
// ============================================================================

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string mode(argv[1]);
        
        if (mode == "--test" || mode == "-t") {
            Calculator::runTests();
        } else if (mode == "--help" || mode == "-h") {
            std::cout << "Использование:\n";
            std::cout << "  " << argv[0] << "           - интерактивный режим\n";
            std::cout << "  " << argv[0] << " --test    - запуск тестов\n";
            std::cout << "  " << argv[0] << " --help    - показать эту справку\n";
            std::cout << "\nПримеры выражений:\n";
            std::cout << "  (3 + 4) * 5\n";
            std::cout << "  10 + 2 * 3\n";
            std::cout << "  3 + 4 * 2 / (1 - 5)\n";
        } else {
            // Попробуем вычислить выражение из аргументов
            std::string expression;
            for (int i = 1; i < argc; i++) {
                if (i > 1) expression += " ";
                expression += argv[i];
            }
            
            try {
                double result = Calculator::calculate(expression);
                std::string prefix = Calculator::getPrefixNotation(expression);
                
                std::cout << "Выражение: " << expression << "\n";
                std::cout << "Польская запись: " << prefix << "\n";
                std::cout << "Результат: " << std::fixed << std::setprecision(6) 
                         << result << "\n";
            } catch (const std::exception& e) {
                std::cerr << "Ошибка: " << e.what() << "\n";
                return 1;
            }
        }
    } else {
        Calculator::runInteractive();
    }
    
    return 0;
}