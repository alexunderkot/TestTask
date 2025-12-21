FROM gcc:latest

# Обновляем и устанавливаем необходимые пакеты
RUN apt-get update && apt-get install -y cmake && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем исходные файлы из корня проекта
COPY ../TestTask.cpp .
COPY ../CMakeLists.txt .
COPY ../build.sh .

# Даем права на выполнение скрипта сборки
RUN chmod +x build.sh

EXPOSE 8080

CMD ["./build.sh"]