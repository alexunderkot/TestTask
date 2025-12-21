FROM alpine:latest

RUN apk update && apk add --no-cache g++ make musl-dev

WORKDIR /workspace

COPY TestTask.cpp /workspace/

RUN g++ -std=c++11 -pthread -O2 TestTask.cpp -o calculator-server

EXPOSE 8080

CMD ["./calculator-server", "8080"]