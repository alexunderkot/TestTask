## Как использовать

# Сборка и запуск:
1. Соберите образ:
docker build -t calculator-system -f .devcontainer/Dockerfile .

2. Запустите сервер в фоне:
docker run -d -p 8080:8080 --name calc-server calculator-system

3. Используйте клиент из контейнера:
docker exec calc-server calc -c echo
docker exec calc-server calc -e "2 + 2"
docker exec calc-server calc -e "(3 + 4) * 5"
