# HW02: Docker образ

## Задание

Создать минимальный сервис, который:
- Отвечает на порту 8000
- Имеет HTTP метод: `GET /health/` → `{"status": "OK"}`

## Решение

Используется разработанная мною библиотека из другого курса Otus "Паттерны проектирования" (по сути - это обертка над Boost.Beast, Boost.DI):
- `microservice-core` - интерфейсы (IHttpHandler, IRequest, IResponse)
- `microservice-boost` - реализация на Boost.Beast (BoostBeastApplication)

## Структура

```
./hw-02-docker/
├── CMakeLists.txt
├── config.json                     # Конфигурация (порт 8000)
├── Dockerfile                      # Multi-stage build
├── include
│   ├── handlers
│   │   └── HealthCheckHandler.hpp  # Handler для /health/
│   └── HelloWorldApp.hpp           # Приложение (наследник BoostBeastApplication)
├── README.md
└── src
    ├── handlers
    │   └── HealthCheckHandler.cpp
    ├── HelloWorldApp.cpp
    └── main.cpp                    # Entry point
```

## Сборка локально

```bash
# Из корня проекта
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Запуск
cd hw-02-docker
./service config.json

# Тест
curl http://localhost:8000/health/
# {"status": "OK"}
```

## Docker

### Сборка образа

```bash
# Из корня проекта
docker build --platform linux/amd64 -t tobantal/otus-hw02:v1 -f hw-02-docker/Dockerfile .
```

### Запуск контейнера

```bash
docker run -d -p 8000:8000 --name hw02 tobantal/otus-hw02:v1
```

### Тестирование

```bash
curl http://localhost:8000/health/
# {"status": "OK"}
```

### Push на DockerHub

```bash
docker login
docker push tobantal/otus-hw02:v1
```

## Результат

- **DockerHub:** `tobantal/otus-hw02:v1`
- **GitHub:** https://github.com/tobantal/Otus-Microservice-Architecture-2025

## API

| Method | Path | Response |
|--------|------|----------|
| GET | /health/ | `{"status": "OK"}` |
| GET | /health | `{"status": "OK"}` |
