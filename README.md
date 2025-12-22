## // Падаван исправил основные проблемы программы с точки зрения читаемости кода, суть программы не меняя
## Как использовать

# Сборка и запуск:

1. Соберите образ:
docker build -t calculator-sessions -f .devcontainer/Dockerfile .

2. Запустите сервер в фоне:
docker run -d -p 8080:8080 --name calc-server calculator-sessions

# Тестирование (также, как и Level 11):
1. Установка переменной для пользователя student
docker exec calc-server calc -u student -e "pi = 3.14"

2. Попытка использовать без указания пользователя (должна быть ошибка)
docker exec calc-server calc -e "2 * pi * 3"
Ожидаемый результат: Error: Unknown variable 'pi'

3. Использование с указанием пользователя
docker exec calc-server calc -u student -e "2 * pi * 3"
Ожидаемый результат: 18.84

4. Очистка сессии пользователя student
docker exec calc-server calc -u student -c clean

5. Проверка очистки
docker exec calc-server calc -u student -e "2 * pi * 3"
Ожидаемый результат: Error: Unknown variable 'pi'

6. Тестирование разных пользователей
docker exec calc-server calc -u alice -e "x = 5"
docker exec calc-server calc -u bob -e "x = 10"
docker exec calc-server calc -u alice -e "x * 2"
Ожидаемый результат: 10

docker exec calc-server calc -u bob -e "x * 2"
Ожидаемый результат: 20
