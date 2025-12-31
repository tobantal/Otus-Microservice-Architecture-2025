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

# Сборка
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target hw05-monitoring -j$(nproc)

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
COPY --from=builder /build/build/hw05-monitoring ./service
COPY config.json .

EXPOSE 8000

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=10s --retries=3 \
    CMD curl -f http://localhost:8000/health || exit 1

CMD ["./service", "config.json"]
