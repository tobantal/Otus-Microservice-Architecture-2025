# Stage 1: Build
FROM ubuntu:22.04 AS builder

# Установка зависимостей для сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libboost-all-dev \
    nlohmann-json3-dev \
    libpq-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Копируем исходники
COPY . .

# Сборка (FetchContent скачает cpp-http-server и libpqxx)
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target hw06-auth -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libpq5 \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем бинарник и конфиг
COPY --from=builder /build/build/hw06-auth ./service
COPY config.json .

# Environment (пароль передаётся через Kubernetes Secret, не здесь!)
ENV DB_HOST=postgres
ENV DB_PORT=5432
ENV DB_NAME=auth
ENV DB_USER=auth
# DB_PASSWORD передаётся через secretKeyRef в deployment.yaml

EXPOSE 8000

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8000/health || exit 1

CMD ["./service", "config.json"]
