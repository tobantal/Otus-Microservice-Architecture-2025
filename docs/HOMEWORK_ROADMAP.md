# HOMEWORK ROADMAP: Микросервисная Архитектура

> **Дата:** 2025-12-30  
> **Курс:** OTUS Microservice Architecture  
> **Студент:** Anton Tobolkin  
> **ОС:** Pop_OS 2024  
> **Инструменты:** Docker, C++ compiler, VS Code  

---

## 📋 Оглавление

1. [Обзор домашних работ](#1-обзор-домашних-работ)
2. [Технологический стек](#2-технологический-стек)
3. [Архитектура проекта](#3-архитектура-проекта)
4. [Git Workflow](#4-git-workflow)
5. [Детальные требования по каждому ДЗ](#5-детальные-требования-по-каждому-дз)
6. [Синергия с курсовой работой](#6-синергия-с-курсовой-работой)
7. [План выполнения и оценка трудозатрат](#7-план-выполнения-и-оценка-трудозатрат)
8. [Риски и митигация](#8-риски-и-митигация)

---

## 1. Обзор домашних работ

| ДЗ | Название | Суть | Ключевые технологии |
|----|----------|------|---------------------|
| **hw02** | Docker образ | /health endpoint → DockerHub | Docker, C++ |
| **hw03** | Kubernetes Basics | K8s манифесты, Ingress | K8s, Helm, nginx-ingress |
| **hw04** | Инфраструктурные паттерны | CRUD Users + PostgreSQL | K8s ConfigMaps, Secrets, Jobs |
| **hw05** | Prometheus & Grafana | Метрики, дашборды, алерты | Prometheus, Grafana |
| **hw06** | API Gateway & Auth | Аутентификация, профили | JWT, API Gateway |
| **hw07** | Stream Processing | Orders + Billing + Notifications | RabbitMQ/Kafka, Events |
| **hw08** | Distributed Transactions | Saga Pattern | Saga, 2PC, Compensation |
| **hw09** | Idempotency | Идемпотентность API | Idempotency Keys |

### Зависимости между ДЗ

```
hw02 (Docker)
  │
  ▼
hw03 (K8s) ──────────────────────────────────────┐
  │                                               │
  ▼                                               │
hw04 (CRUD + DB) ─────────────────────────────────┤
  │                                               │
  ├─────────────────┬─────────────────────────────┤
  ▼                 ▼                             │
hw05 (Metrics)    hw06 (Auth)                     │
                    │                             │
                    ▼                             │
                  hw07 (Orders + Billing + Notif) │
                    │                             │
                    ▼                             │
                  hw08 (Saga) ────────────────────┘
                    │
                    ▼
                  hw09 (Idempotency)
```

---

## 2. Технологический стек

### 2.1 Уже установлено/знакомо ✅

| Технология | Уровень | Примечание |
|------------|---------|------------|
| Docker | ✅ Работает | docker, docker-compose |
| C++ Compiler | ✅ Работает | g++, CMake |
| VS Code | ✅ Работает | IDE |
| Kubernetes | ⚠️ Базово | minikube/kind нужно установить |
| Prometheus | ⚠️ Вчера | Базовое понимание |
| Grafana | ⚠️ Вчера | Базовое понимание |

### 2.2 Нужно изучить/установить 📚

| Технология | Для ДЗ | Сложность | Время изучения |
|------------|--------|-----------|----------------|
| **minikube** | hw03+ | Низкая | 1 час |
| **kubectl** | hw03+ | Низкая | 1 час |
| **Helm** | hw03+ | Средняя | 2 часа |
| **nginx-ingress** | hw03+ | Средняя | 1 час |
| **Postman + Newman** | hw04+ | Низкая | 30 мин |
| **PostgreSQL (в K8s)** | hw04+ | Средняя | 1 час |

### 2.3 Команды установки

```bash
# Minikube (для Pop_OS/Ubuntu)
curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64
sudo install minikube-linux-amd64 /usr/local/bin/minikube

# Kubectl
curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"
sudo install kubectl /usr/local/bin/kubectl

# Helm
curl https://raw.githubusercontent.com/helm/helm/main/scripts/get-helm-3 | bash

# Newman (для тестов Postman)
npm install -g newman

# Запуск minikube
minikube start --driver=docker

# Установка nginx-ingress
kubectl create namespace m
helm repo add ingress-nginx https://kubernetes.github.io/ingress-nginx/
helm repo update
helm install nginx ingress-nginx/ingress-nginx --namespace m

# Добавить в /etc/hosts
echo "$(minikube ip) arch.homework" | sudo tee -a /etc/hosts
```

---

## 3. Архитектура проекта

### 3.1 Структура репозитория

```
otus-microservices-homework/
├── CMakeLists.txt              # Root CMake
├── README.md                   # Общее описание
├── HOMEWORK_ROADMAP.md         # Этот файл
│
├── common/                     # Общие компоненты
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── health/
│   │   │   └── HealthHandler.hpp
│   │   ├── http/
│   │   │   └── (SimpleRequest, SimpleResponse, HttpClient)
│   │   └── domain/
│   │       └── User.hpp
│   └── src/
│
├── hw02-docker/                # ДЗ 02: Docker
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   ├── README.md
│   ├── src/
│   │   └── main.cpp
│   └── config.json
│
├── hw03-kubernetes/            # ДЗ 03: Kubernetes
│   ├── k8s/
│   │   ├── deployment.yaml
│   │   ├── service.yaml
│   │   └── ingress.yaml
│   ├── README.md
│   └── postman/
│       └── hw03-health.postman_collection.json
│
├── hw04-crud/                  # ДЗ 04: CRUD + PostgreSQL
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   ├── k8s/
│   │   ├── configmap.yaml
│   │   ├── secret.yaml
│   │   ├── deployment.yaml
│   │   ├── service.yaml
│   │   ├── ingress.yaml
│   │   └── migration-job.yaml
│   ├── helm/
│   │   └── values-postgres.yaml
│   ├── sql/
│   │   └── init.sql
│   ├── include/
│   ├── src/
│   ├── README.md
│   └── postman/
│
├── hw05-monitoring/            # ДЗ 05: Prometheus + Grafana
│   ├── grafana/
│   │   └── dashboards/
│   │       └── service-dashboard.json
│   ├── prometheus/
│   │   └── alerts.yaml
│   ├── screenshots/
│   ├── README.md
│   └── stress-test.sh
│
├── hw06-auth/                  # ДЗ 06: Authentication
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   ├── k8s/
│   ├── include/
│   │   ├── auth/
│   │   └── profile/
│   ├── src/
│   ├── README.md
│   ├── ARCHITECTURE.md         # Схема взаимодействия
│   └── postman/
│
├── hw07-events/                # ДЗ 07: Event-driven
│   ├── services/
│   │   ├── order-service/
│   │   ├── billing-service/
│   │   └── notification-service/
│   ├── k8s/
│   ├── README.md
│   ├── ARCHITECTURE.md
│   └── postman/
│
├── hw08-saga/                  # ДЗ 08: Distributed Transactions
│   ├── services/
│   │   ├── order-service/      # Расширение из hw07
│   │   ├── billing-service/    # Расширение из hw07
│   │   ├── warehouse-service/  # НОВЫЙ
│   │   └── delivery-service/   # НОВЫЙ
│   ├── k8s/
│   ├── README.md
│   ├── SAGA_PATTERN.md
│   └── postman/
│
└── hw09-idempotency/           # ДЗ 09: Idempotency
    ├── README.md
    ├── IDEMPOTENCY_PATTERN.md
    └── postman/
```

### 3.2 Переиспользование кода из курсовой (MVP)

| Компонент MVP | Используется в ДЗ | Адаптация |
|---------------|-------------------|-----------|
| `HealthHandler` | hw02, hw03 | Минимальная (порт 8000) |
| `BoostBeastApplication` | Все | Без изменений |
| `InMemoryUserRepository` | hw04, hw06 | Добавить PostgreSQL |
| `AuthService` | hw06 | Упростить (только login/register) |
| `OrderService` | hw07, hw08 | Упростить domain model |
| `RabbitMQEventBus` | hw07, hw08 | Без изменений |
| `Prometheus Metrics` | hw05 | Добавить квантили |
| `FakeJwtAdapter` | hw06 | Без изменений |

---

## 4. Git Workflow

### 4.1 Стратегия веток

```
main
 │
 ├── hw02 ────────────────────── PR #1 → main
 │    │
 │    └── hw03 ─────────────────── PR #2 → hw02
 │         │
 │         └── hw04 ──────────────── PR #3 → hw03
 │              │
 │              ├── hw05 ───────────── PR #4 → hw04
 │              │
 │              └── hw06 ───────────── PR #5 → hw04
 │                   │
 │                   └── hw07 ────────── PR #6 → hw06
 │                        │
 │                        └── hw08 ─────── PR #7 → hw07
 │                             │
 │                             └── hw09 ──── PR #8 → hw08
```

### 4.2 Команды для работы

```bash
# Начало работы над hw02
git checkout main
git checkout -b hw02
# ... работа ...
git push origin hw02
# Создать PR: hw02 → main

# Переход к hw03 (не дожидаясь мержа hw02)
git checkout hw02
git checkout -b hw03
# ... работа ...
git push origin hw03
# Создать PR: hw03 → hw02

# И так далее...
```

### 4.3 Преимущества подхода

- ✅ Можно сдавать ДЗ параллельно
- ✅ Преподаватели проверяют независимо
- ✅ Изменения каскадно мержатся
- ✅ Каждое ДЗ видит код предыдущих

---

## 5. Детальные требования по каждому ДЗ

---

### 📦 HW02: Docker образ

**Цель:** Обернуть приложение в Docker и запушить на DockerHub

#### Требования

| # | Требование | Статус |
|---|------------|--------|
| 1 | Сервис отвечает на порту **8000** | ⬜ |
| 2 | `GET /health/` → `{"status": "OK"}` | ⬜ |
| 3 | Docker образ собран для **linux/amd64** | ⬜ |
| 4 | Образ запушен на DockerHub | ⬜ |

#### Что сдаём

- [ ] Имя репозитория и тег на DockerHub
- [ ] Ссылка на GitHub с Dockerfile

#### Реализация (из MVP)

```cpp
// Переиспользуем HealthHandler из MVP
// Только меняем порт на 8000
class HealthHandler : public IHttpHandler {
public:
    void handle(IRequest& req, IResponse& res) override {
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(R"({"status": "OK"})");
    }
};
```

#### Dockerfile

```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY build/hw02-docker /app/service
COPY config.json /app/
EXPOSE 8000
CMD ["./service"]
```

#### Команды

```bash
# Сборка
docker build --platform linux/amd64 -t tobantal/otus-hw02:v1 .

# Пуш
docker push tobantal/otus-hw02:v1

# Тест
docker run -p 8000:8000 tobantal/otus-hw02:v1
curl http://localhost:8000/health/
```

#### ⏱ Оценка: **1-2 часа**

---

### ☸️ HW03: Kubernetes Basics

**Цель:** Развернуть сервис в Kubernetes

#### Требования

| # | Требование | Статус |
|---|------------|--------|
| 1 | Манифесты: Deployment, Service, Ingress | ⬜ |
| 2 | Liveness/Readiness probes | ⬜ |
| 3 | Минимум 2 реплики | ⬜ |
| 4 | Хост: `arch.homework` | ⬜ |
| 5 | `GET http://arch.homework/health` → OK | ⬜ |
| 6 | ⭐ Rewrite `/otusapp/{name}/*` → `/*` | ⬜ |

#### Что сдаём

- [ ] GitHub с манифестами (применяются одной командой)
- [ ] URL для проверки или Postman тест

#### Манифесты

**deployment.yaml**
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: hw03-service
spec:
  replicas: 2
  selector:
    matchLabels:
      app: hw03-service
  template:
    metadata:
      labels:
        app: hw03-service
    spec:
      containers:
      - name: service
        image: tobantal/otus-hw02:v1
        ports:
        - containerPort: 8000
        livenessProbe:
          httpGet:
            path: /health/
            port: 8000
          initialDelaySeconds: 5
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /health/
            port: 8000
          initialDelaySeconds: 3
          periodSeconds: 5
```

**service.yaml**
```yaml
apiVersion: v1
kind: Service
metadata:
  name: hw03-service
spec:
  selector:
    app: hw03-service
  ports:
  - port: 80
    targetPort: 8000
```

**ingress.yaml**
```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: hw03-ingress
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /$2
spec:
  ingressClassName: nginx
  rules:
  - host: arch.homework
    http:
      paths:
      - path: /health
        pathType: Prefix
        backend:
          service:
            name: hw03-service
            port:
              number: 80
      - path: /otusapp/tobolkin(/|$)(.*)
        pathType: ImplementationSpecific
        backend:
          service:
            name: hw03-service
            port:
              number: 80
```

#### Команды

```bash
# Применить манифесты
kubectl apply -f k8s/

# Проверить
curl http://arch.homework/health
curl http://arch.homework/otusapp/tobolkin/health
```

#### ⏱ Оценка: **2-3 часа**

---

### 🗄️ HW04: CRUD + PostgreSQL

**Цель:** RESTful CRUD для Users с базой данных

#### API Endpoints

| Method | Path | Описание |
|--------|------|----------|
| POST | /api/v1/users | Создать пользователя |
| GET | /api/v1/users/{id} | Получить пользователя |
| PUT | /api/v1/users/{id} | Обновить пользователя |
| DELETE | /api/v1/users/{id} | Удалить пользователя |
| GET | /api/v1/users | Список пользователей |

#### User Model

```json
{
  "id": 1,
  "username": "johndoe",
  "firstName": "John",
  "lastName": "Doe",
  "email": "john@example.com",
  "phone": "+1234567890"
}
```

#### Требования

| # | Требование | Статус |
|---|------------|--------|
| 1 | CRUD API по спецификации | ⬜ |
| 2 | PostgreSQL для хранения | ⬜ |
| 3 | ConfigMap для конфигурации | ⬜ |
| 4 | Secret для credentials БД | ⬜ |
| 5 | Job для миграций | ⬜ |
| 6 | Ingress на `arch.homework` | ⬜ |
| 7 | Postman коллекция + newman | ⬜ |

#### Что сдаём

- [ ] GitHub с манифестами
- [ ] Инструкция по запуску
- [ ] Helm values.yaml для PostgreSQL
- [ ] Postman коллекция
- [ ] Скриншот newman run

#### Переиспользование из MVP

```cpp
// Адаптируем InMemoryUserRepository → PostgresUserRepository
// Domain: User уже есть
// Handlers: аналогичны нашим Auth handlers
```

#### Команды

```bash
# Установка PostgreSQL через Helm
helm repo add bitnami https://charts.bitnami.com/bitnami
helm install postgres bitnami/postgresql -f helm/values-postgres.yaml

# Миграции
kubectl apply -f k8s/migration-job.yaml

# Приложение
kubectl apply -f k8s/

# Тесты
newman run postman/hw04-users.postman_collection.json
```

#### ⏱ Оценка: **4-5 часов**

---

### 📊 HW05: Prometheus & Grafana

**Цель:** Метрики и дашборды

#### Метрики (по API методам)

| Метрика | Описание |
|---------|----------|
| Latency | p50, p95, p99, max |
| RPS | Requests per second |
| Error Rate | Количество 5xx |

#### Требования

| # | Требование | Статус |
|---|------------|--------|
| 1 | Метрики с разбивкой по методам | ⬜ |
| 2 | Grafana дашборд | ⬜ |
| 3 | Метрики с nginx-ingress | ⬜ |
| 4 | Алерты на Error Rate и Latency | ⬜ |
| 5 | Скриншоты под нагрузкой | ⬜ |

#### Что сдаём

- [ ] Скриншоты дашборда (5-10 мин нагрузки)
- [ ] JSON дашборда

#### Метрики в коде (уже есть в MVP!)

```cpp
// В MetricsHandler уже есть базовые метрики
// Нужно добавить гистограммы с квантилями

// Пример Prometheus output:
// http_request_duration_seconds{method="GET",path="/users",quantile="0.5"} 0.023
// http_request_duration_seconds{method="GET",path="/users",quantile="0.95"} 0.089
// http_requests_total{method="GET",path="/users",status="200"} 1523
// http_requests_total{method="GET",path="/users",status="500"} 12
```

#### Stress Test

```bash
#!/bin/bash
# stress-test.sh
ab -n 10000 -c 50 http://arch.homework/api/v1/users/1
```

#### ⏱ Оценка: **3-4 часа**

---

### 🔐 HW06: Authentication & API Gateway

**Цель:** Аутентификация и защита профилей

#### Сценарий

1. Пользователь регистрируется
2. Пользователь логинится → получает токен
3. Пользователь видит/редактирует ТОЛЬКО свой профиль
4. Другие пользователи НЕ видят чужие профили

#### API Endpoints

| Method | Path | Описание | Auth |
|--------|------|----------|------|
| POST | /api/v1/auth/register | Регистрация | ❌ |
| POST | /api/v1/auth/login | Вход | ❌ |
| POST | /api/v1/auth/logout | Выход | ✅ |
| GET | /api/v1/profile | Мой профиль | ✅ |
| PUT | /api/v1/profile | Изменить профиль | ✅ |

#### Требования

| # | Требование | Статус |
|---|------------|--------|
| 1 | Схема архитектуры (картинка) | ⬜ |
| 2 | Регистрация/Логин работают | ⬜ |
| 3 | Профиль защищён токеном | ⬜ |
| 4 | User1 не видит профиль User2 | ⬜ |
| 5 | Postman тесты (8 шагов) | ⬜ |

#### Postman сценарий

1. ✅ Регистрация user1
2. ✅ Проверка: GET /profile без токена → 401
3. ✅ Логин user1 → token
4. ✅ PUT /profile (изменение)
5. ✅ GET /profile (проверка изменений)
6. ✅ Logout (опционально)
7. ✅ Регистрация user2
8. ✅ Логин user2
9. ✅ Проверка: user2 не видит профиль user1

#### Переиспользование из MVP

```cpp
// AuthService, FakeJwtAdapter - уже есть!
// LoginHandler, RegisterHandler - уже есть!
// Нужно добавить ProfileHandler
```

#### ⏱ Оценка: **4-5 часов**

---

### 📨 HW07: Event-Driven Architecture

**Цель:** Orders + Billing + Notifications через события

#### Сервисы

| Сервис | Ответственность |
|--------|-----------------|
| **Order Service** | Создание заказов |
| **Billing Service** | Счета, пополнение, списание |
| **Notification Service** | Сохранение/получение уведомлений |

#### Сценарий

```
┌──────────────────────────────────────────────────────────────────┐
│                    УСПЕШНЫЙ ЗАКАЗ                                 │
└──────────────────────────────────────────────────────────────────┘

  Client          Order           Billing         Notification
    │               │                │                 │
    │ POST /orders  │                │                 │
    │──────────────▶│                │                 │
    │               │                │                 │
    │               │ Charge(amount) │                 │
    │               │───────────────▶│                 │
    │               │                │                 │
    │               │    Success     │                 │
    │               │◀───────────────│                 │
    │               │                │                 │
    │               │                │  SendEmail     │
    │               │────────────────────────────────▶│
    │               │                │                 │ (Письмо счастья)
    │  201 Created  │                │                 │
    │◀──────────────│                │                 │


┌──────────────────────────────────────────────────────────────────┐
│                    НЕДОСТАТОЧНО СРЕДСТВ                           │
└──────────────────────────────────────────────────────────────────┘

  Client          Order           Billing         Notification
    │               │                │                 │
    │ POST /orders  │                │                 │
    │──────────────▶│                │                 │
    │               │                │                 │
    │               │ Charge(amount) │                 │
    │               │───────────────▶│                 │
    │               │                │                 │
    │               │  Insufficient  │                 │
    │               │◀───────────────│                 │
    │               │                │                 │
    │               │                │  SendEmail     │
    │               │────────────────────────────────▶│
    │               │                │                 │ (Письмо горя)
    │  400 Failed   │                │                 │
    │◀──────────────│                │                 │
```

#### API Endpoints

**Billing Service:**
| Method | Path | Описание |
|--------|------|----------|
| POST | /api/v1/billing/accounts | Создать аккаунт |
| POST | /api/v1/billing/deposit | Пополнить |
| POST | /api/v1/billing/charge | Списать |
| GET | /api/v1/billing/accounts/{userId} | Баланс |

**Notification Service:**
| Method | Path | Описание |
|--------|------|----------|
| POST | /api/v1/notifications | Отправить |
| GET | /api/v1/notifications?userId={id} | Список сообщений |

**Order Service:**
| Method | Path | Описание |
|--------|------|----------|
| POST | /api/v1/orders | Создать заказ |
| GET | /api/v1/orders/{id} | Получить заказ |

#### Варианты реализации (теория)

1. **HTTP only** - синхронные вызовы
2. **Events для notifications** - HTTP + RabbitMQ для писем
3. **Event Collaboration** - всё через события
4. **Рекомендуемый** - вариант 2 (баланс простоты и асинхронности)

#### Postman сценарий

1. ✅ Создать пользователя → аккаунт в billing
2. ✅ Пополнить счёт
3. ✅ Создать заказ (хватает денег)
4. ✅ Проверить баланс (уменьшился)
5. ✅ Проверить notification (письмо счастья)
6. ✅ Создать заказ (не хватает денег)
7. ✅ Проверить баланс (не изменился)
8. ✅ Проверить notification (письмо горя)

#### Переиспользование из MVP

```cpp
// RabbitMQEventBus - уже есть!
// OrderService - адаптируем
// Billing ≈ наш Portfolio (баланс)
// Notifications - новый простой сервис
```

#### ⏱ Оценка: **6-8 часов**

---

### 🔄 HW08: Distributed Transactions (Saga)

**Цель:** Saga Pattern для распределённых транзакций

#### Сервисы

| Сервис | Ответственность | Компенсация |
|--------|-----------------|-------------|
| **Billing** | Списание денег | Возврат денег |
| **Warehouse** | Резервирование товара | Снятие резерва |
| **Delivery** | Бронирование курьера | Отмена брони |

#### Сценарий Saga

```
┌──────────────────────────────────────────────────────────────────┐
│                    SAGA: СОЗДАНИЕ ЗАКАЗА                          │
└──────────────────────────────────────────────────────────────────┘

  Order          Billing        Warehouse       Delivery
    │               │               │               │
    │ 1. Charge     │               │               │
    │──────────────▶│               │               │
    │      OK       │               │               │
    │◀──────────────│               │               │
    │               │               │               │
    │ 2. Reserve    │               │               │
    │──────────────────────────────▶│               │
    │      OK       │               │               │
    │◀──────────────────────────────│               │
    │               │               │               │
    │ 3. Book       │               │               │
    │──────────────────────────────────────────────▶│
    │      OK       │               │               │
    │◀──────────────────────────────────────────────│
    │               │               │               │
    │ ✅ SUCCESS    │               │               │


┌──────────────────────────────────────────────────────────────────┐
│                    SAGA: ОТКАТ (Delivery failed)                  │
└──────────────────────────────────────────────────────────────────┘

  Order          Billing        Warehouse       Delivery
    │               │               │               │
    │ 1. Charge     │               │               │
    │──────────────▶│               │               │
    │      OK       │               │               │
    │◀──────────────│               │               │
    │               │               │               │
    │ 2. Reserve    │               │               │
    │──────────────────────────────▶│               │
    │      OK       │               │               │
    │◀──────────────────────────────│               │
    │               │               │               │
    │ 3. Book       │               │               │
    │──────────────────────────────────────────────▶│
    │    FAILED!    │               │               │
    │◀──────────────────────────────────────────────│
    │               │               │               │
    │ COMPENSATE:   │               │               │
    │               │               │               │
    │ 2c. Release   │               │               │
    │──────────────────────────────▶│               │
    │◀──────────────────────────────│               │
    │               │               │               │
    │ 1c. Refund    │               │               │
    │──────────────▶│               │               │
    │◀──────────────│               │               │
    │               │               │               │
    │ ❌ ROLLBACK   │               │               │
```

#### Требования

| # | Требование | Статус |
|---|------------|--------|
| 1 | Описание паттерна (Saga) | ⬜ |
| 2 | Billing, Warehouse, Delivery сервисы | ⬜ |
| 3 | Компенсирующие транзакции | ⬜ |
| 4 | Helm/манифесты | ⬜ |
| 5 | Postman тесты | ⬜ |

#### Переиспользование из MVP

```cpp
// ROADMAP_v3 уже описывает Saga для Trading!
// OrderProcessor с pending orders - похоже на Saga state
// Компенсации уже продуманы
```

#### ⏱ Оценка: **6-8 часов**

---

### 🔑 HW09: Idempotency

**Цель:** Сделать "создание заказа" идемпотентным

#### Паттерн: Idempotency Key

```
Client                          Order Service
  │                                   │
  │ POST /orders                      │
  │ X-Idempotency-Key: abc-123        │
  │───────────────────────────────────▶│
  │                                   │
  │     201 Created (order_id: 42)    │
  │◀───────────────────────────────────│
  │                                   │
  │ POST /orders                      │
  │ X-Idempotency-Key: abc-123        │ (retry)
  │───────────────────────────────────▶│
  │                                   │
  │     200 OK (order_id: 42)         │ (тот же результат!)
  │◀───────────────────────────────────│
```

#### Реализация

```cpp
// Храним в Redis/DB:
// idempotency_key → {status, response, created_at}

class IdempotencyMiddleware {
    bool checkIdempotencyKey(const std::string& key, IResponse& cachedResponse) {
        // Проверяем в кеше/БД
        // Если есть - возвращаем сохранённый ответ
        // Если нет - пропускаем дальше
    }
    
    void saveIdempotencyKey(const std::string& key, const IResponse& response) {
        // Сохраняем результат
    }
};
```

#### Требования

| # | Требование | Статус |
|---|------------|--------|
| 1 | Описание паттерна | ⬜ |
| 2 | Реализация для POST /orders | ⬜ |
| 3 | Helm/манифесты | ⬜ |
| 4 | Postman тесты | ⬜ |

#### ⏱ Оценка: **2-3 часа** (расширение hw08)

---

## 6. Синергия с курсовой работой

### 🔄 Что из ДЗ полезно для курсовой

| ДЗ | Полезность для курсовой | Комментарий |
|----|-------------------------|-------------|
| hw02 | ⭐⭐⭐ | Dockerfile уже есть |
| hw03 | ⭐⭐⭐⭐⭐ | K8s манифесты для микросервисов |
| hw04 | ⭐⭐⭐ | PostgreSQL уже настроен |
| hw05 | ⭐⭐⭐⭐⭐ | Grafana дашборды для trading |
| hw06 | ⭐⭐⭐⭐⭐ | AuthService почти готов |
| hw07 | ⭐⭐⭐⭐⭐ | RabbitMQ + Events = наша архитектура! |
| hw08 | ⭐⭐⭐⭐⭐ | Saga для Order → Broker |
| hw09 | ⭐⭐⭐⭐ | Идемпотентность для trading |

### 🔄 Что из курсовой полезно для ДЗ

| Компонент курсовой | Используется в ДЗ |
|--------------------|-------------------|
| HealthHandler | hw02, hw03 |
| AuthService + JWT | hw06 |
| OrderService | hw07, hw08, hw09 |
| RabbitMQEventBus | hw07, hw08 |
| PostgresRepositories | hw04 |
| Prometheus metrics | hw05 |
| Saga (в ROADMAP_v3) | hw08 |

### 💡 Переосмысление курсовой

После изучения ДЗ, предлагаю изменить приоритеты:

1. **Сначала hw02-hw05** — базовая инфраструктура (Docker, K8s, DB, Monitoring)
2. **Параллельно** — Auth Service для курсовой (hw06 почти идентичен)
3. **hw07-hw08** — делаем на основе курсовой (Trading ≈ Orders, Broker ≈ Billing)
4. **hw09** — добавляем idempotency в Trading

**Итог:** Делая ДЗ, мы автоматически делаем курсовую!

---

## 7. План выполнения и оценка трудозатрат

### Сводная таблица

| ДЗ | Оценка | Переиспользование | Новый код | Риск |
|----|--------|-------------------|-----------|------|
| hw02 | 1-2 ч | 90% | Dockerfile | 🟢 Низкий |
| hw03 | 2-3 ч | 70% | K8s манифесты | 🟡 Средний |
| hw04 | 4-5 ч | 60% | PostgreSQL, CRUD | 🟡 Средний |
| hw05 | 3-4 ч | 50% | Grafana дашборды | 🟡 Средний |
| hw06 | 4-5 ч | 80% | Profile handler | 🟢 Низкий |
| hw07 | 6-8 ч | 60% | Billing, Notif сервисы | 🟡 Средний |
| hw08 | 6-8 ч | 40% | Saga, Warehouse, Delivery | 🔴 Высокий |
| hw09 | 2-3 ч | 80% | Idempotency middleware | 🟢 Низкий |

### **ИТОГО: 29-38 часов**

### Рекомендуемый порядок

```
Неделя 1 (быстрые ДЗ):
├── hw02 (1-2ч) ─────────────────────── День 1, утро
├── hw03 (2-3ч) ─────────────────────── День 1, день
└── hw04 (4-5ч) ─────────────────────── День 2

Неделя 2 (средние ДЗ):
├── hw05 (3-4ч) ─────────────────────── День 3
├── hw06 (4-5ч) ─────────────────────── День 4
└── Начало курсовой (распил) ────────── День 5

Неделя 3 (сложные ДЗ + курсовая):
├── hw07 (6-8ч) ─────────────────────── Дни 6-7
├── hw08 (6-8ч) ─────────────────────── Дни 8-9
└── hw09 (2-3ч) ─────────────────────── День 10
```

---

## 8. Риски и митигация

| Риск | Вероятность | Митигация |
|------|-------------|-----------|
| Minikube не работает | Средняя | Использовать kind или Docker Desktop с K8s |
| Helm charts конфликтуют | Средняя | Отдельные namespace для каждого ДЗ |
| PostgreSQL в K8s сложнее | Средняя | Использовать managed DB или StatefulSet |
| Не хватает времени на hw08 | Высокая | Упростить до 2 сервисов вместо 4 |
| Newman тесты падают | Низкая | Добавить retries и delays |

---

## 9. Решение: Что делать сейчас?

### Рекомендация: **Начать с ДЗ hw02-hw04**

**Причины:**
1. ✅ Быстрые (7-10 часов на 3 ДЗ)
2. ✅ Создают инфраструктуру для курсовой
3. ✅ Minikube + Helm пригодятся для деплоя микросервисов
4. ✅ PostgreSQL в K8s = готовая БД для курсовой
5. ✅ Можно сдавать параллельно

### Альтернатива: **Начать с распила курсовой**

**Если курсовая приоритетнее:**
1. Распил монолита (10-12 часов)
2. Потом hw02-hw04 быстро (используя готовые сервисы)
3. hw05-hw09 на основе курсовой

---

## 📋 Чеклист для начала

- [ ] Установить minikube
- [ ] Установить kubectl
- [ ] Установить Helm
- [ ] Установить newman
- [ ] Добавить `arch.homework` в /etc/hosts
- [ ] Создать репозиторий `otus-microservices-homework`
- [ ] Создать ветку `hw02`
- [ ] Начать работу!

---

**Вопрос:** Начинаем с hw02-hw04 или с распила курсовой?
