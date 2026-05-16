FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 1. Instala o compilador, o Postgres E o "curl" ou "wget" para baixar o Crow
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    libpq-dev \
    libpqxx-dev \
    curl \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 2. Baixa o Crow (Essa parte já funcionou!)
RUN curl -L https://github.com/CrowCpp/Crow/releases/download/v1.0+5/crow_all.h -o crow.h

COPY . .

# 3. ALTERADO: Adicionado -lpqxx para linkar a biblioteca C++ do Postgres
RUN g++ main.cpp -o servidor -lpqxx -lpq -pthread

EXPOSE 10000

CMD ["./servidor"]