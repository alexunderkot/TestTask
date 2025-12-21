FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

RUN apt-get update && apt-get install -y \
    cmake \
    gdb \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

WORKDIR /workspace/build
RUN cmake .. && make

CMD ["./TestTask"]