# 1. Escolhemos o "computador" virtual (Ubuntu)
FROM ubuntu:22.04

# 2. Evita perguntas interativas durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# 3. Instala compilador e bibliotecas do PostgreSQL
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# 4. Define onde o código ficará dentro do container
WORKDIR /app

# 5. Copia seus arquivos do Windows para dentro do container
COPY . .

# 6. Compila o código (Ajuste o comando conforme seu projeto)
# Exemplo se for um único arquivo main.cpp usando a biblioteca do Postgres
RUN g++ main.cpp -o servidor -lpq

# 7. Expõe a porta que o Render usa
EXPOSE 10000

# 8. Comando para iniciar o seu encurtador
CMD ["./servidor"]