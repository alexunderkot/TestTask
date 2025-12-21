# Собрать образ
docker build -t calculator-server -f .devcontainer/Dockerfile .

# Запустить сервер
docker run -p 8080:8080 calc-server

# Далее есть два варианта взаимодействия:
1) Перейти по ссылке (http://localhost:8080/ - стандартная) в браузере, пользоваться        калькулятором

2) Через консоль

    # Вычислить 2+2
curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"exp\":\"2 + 2\"}"

# С приоритетами
curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"exp\":\"2 + 3 * 4\"}"

   # Со скобками
curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"exp\":\"(3 + 4) * 5\"}"

# Вещественные числа
curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"exp\":\"3.5 + 2.5\"}"

# Сложные выражения
curl -X POST http://localhost:8080/ -H "Content-Type: application/json" -d "{\"exp\":\"3 + 4 * 2 / (1 - 5)\"}"