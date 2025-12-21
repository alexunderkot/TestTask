## Как использовать

# Сборка и запуск:
1. Соберите образ:
docker build -t calculator-vars -f .devcontainer/Dockerfile .

2. Запустите сервер в фоне:
docker run -d -p 8080:8080 --name calc-server calculator-vars

3. Тестирование:
# Простое присваивание и использование
docker exec calc-server calc -e "var = 2 + 3; var * 2"
Результат: 10

# Ошибка: неизвестная переменная
docker exec calc-server calc -e "foo = 2 + 3; bar * 2"
Результат: Error: Unknown variable 'bar'

# Многострочный ввод
docker exec -it calc-server sh -c "calc -e"
# Затем вводите построчно:
var = 2 + 3
var * 2
[пустая строка]

# 3. Тестируйте Level 10
docker exec calc-server calc -e "pi = 3.14"
Результат: (пустой вывод)

docker exec calc-server calc -e "2 * pi * 3"
Результат: 18.84

docker exec calc-server calc -c clean
Результат: (пустой вывод)

docker exec calc-server calc -e "2 * pi * 3"
Результат: Error: Unknown variable 'pi'

# 4. Тестируйте через curl
curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"exp\":\"var = 2 + 3; var * 2\"}"
Результат: {"res":10}

curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"cmd\":\"clean\"}"
Результат: {}

curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"exp\":\"pi = 3.14\"}"
Результат: {}