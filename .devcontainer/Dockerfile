FROM ubuntu:22.04

# Установка необходимых пакетов
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    g++ \
    build-essential \
    git \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Создание рабочей директории
WORKDIR /workspace

# Копирование исходного кода из родительской директории
COPY TestTask.cpp /workspace/TestTask.cpp

# Компиляция программы
RUN g++ -std=c++17 -Wall -Wextra -O2 TestTask.cpp -o calculator

# Точка входа для запуска калькулятора
CMD ["./calculator"]