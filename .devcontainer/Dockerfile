FROM alpine:latest

RUN apk update && apk add --no-cache g++ make musl-dev curl

WORKDIR /workspace

COPY . .

RUN g++ -std=c++17 -pthread -I./include \
    server/main_server.cpp \
    src/Calculator.cpp \
    src/ExpressionParser.cpp \
    src/JsonParser.cpp \
    src/HttpServer.cpp \
    src/SessionStore.cpp \
    -o /usr/local/bin/calculator-server

RUN echo "Checking for client files..." && \
    ls -la client/ && \
    ls -la src/CalculatorClient.cpp src/HttpClient.cpp src/JsonHandler.cpp 2>/dev/null || echo "Some client files missing"

RUN if [ -f client/main_client.cpp ] && [ -f src/CalculatorClient.cpp ] && [ -f src/HttpClient.cpp ] && [ -f src/JsonHandler.cpp ]; then \
    echo "Compiling client..." && \
    g++ -std=c++17 -I./include \
        client/main_client.cpp \
        src/CalculatorClient.cpp \
        src/HttpClient.cpp \
        src/JsonHandler.cpp \
        -o /usr/local/bin/calc; \
    echo "Client compiled successfully"; \
else \
    echo "Creating simple bash client..." && \
    echo '#!/bin/sh' > /usr/local/bin/calc && \
    echo 'if [ "$1" = "-u" ]; then' >> /usr/local/bin/calc && \
    echo '    USER="$2"' >> /usr/local/bin/calc && \
    echo '    shift 2' >> /usr/local/bin/calc && \
    echo 'else' >> /usr/local/bin/calc && \
    echo '    USER=""' >> /usr/local/bin/calc && \
    echo 'fi' >> /usr/local/bin/calc && \
    echo '' >> /usr/local/bin/calc && \
    echo 'if [ "$1" = "-e" ]; then' >> /usr/local/bin/calc && \
    echo '    EXPR="$2"' >> /usr/local/bin/calc && \
    echo '    JSON="{\"exp\": \"$EXPR\""' >> /usr/local/bin/calc && \
    echo '    if [ -n "$USER" ]; then' >> /usr/local/bin/calc && \
    echo '        JSON="$JSON, \"user\": \"$USER\""' >> /usr/local/bin/calc && \
    echo '    fi' >> /usr/local/bin/calc && \
    echo '    JSON="$JSON}"' >> /usr/local/bin/calc && \
    echo '    curl -s -X POST http://localhost:8080 \' >> /usr/local/bin/calc && \
    echo '        -H "Content-Type: application/json" \' >> /usr/local/bin/calc && \
    echo '        -d "$JSON"' >> /usr/local/bin/calc && \
    echo 'fi' >> /usr/local/bin/calc && \
    chmod +x /usr/local/bin/calc; \
    echo "Bash client created"; \
fi

EXPOSE 8080

CMD ["calculator-server", "8080"]