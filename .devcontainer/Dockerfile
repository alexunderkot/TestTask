FROM alpine:latest

RUN apk update && apk add --no-cache g++ make musl-dev curl

WORKDIR /workspace

COPY src/ /workspace/src/

RUN g++ -std=c++17 -pthread -O2 src/server.cpp -o /usr/local/bin/calc-server

RUN g++ -std=c++17 -O2 src/client.cpp -o /usr/local/bin/calc

EXPOSE 8080

CMD ["calc-server", "8080"]