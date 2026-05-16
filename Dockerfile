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

# 2. Baixa o arquivo do Crow direto do GitHub oficial para a pasta do build
# Usamos a versão estável mais comum do arquivo único (crow_all.h)
RUN curl -L https://github.com/CrowCpp/Crow/releases/download/v1.0+5/crow_all.h -o crow.h

# 3. Copia os seus arquivos do Windows (incluindo o main.cpp)
COPY . .

# 4. Compila o código linkando o Postgres
RUN g++ main.cpp -o servidor -lpq -lpqxx -pthread

EXPOSE 10000

CMD ["./servidor"]