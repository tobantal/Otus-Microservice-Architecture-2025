# HW-07: Event-Driven Architecture

Домашнее задание по теме "Событийная архитектура" (Event-Driven Architecture).

## Описание

Реализация межсервисного взаимодействия через HTTP (синхронные вызовы) между тремя микросервисами:

- **Order Service** (порт 8000) — координатор, создаёт заказы
- **Billing Service** (порт 8001) — управление балансами пользователей
- **Notification Service** (порт 8002) — сохранение уведомлений

## Сценарий

```
┌────────┐     POST /orders       ┌───────────────┐
│ Client │ ──────────────────────▶│ Order Service │
└────────┘                        └───────┬───────┘
                                          │
                    ┌─────────────────────┼─────────────────────┐
                    │                     │                     │
                    ▼                     ▼                     ▼
           POST /charge          POST /notifications    Save to DB
           ┌─────────────┐       ┌────────────────────┐
           │   Billing   │       │   Notification     │
           │   Service   │       │   Service          │
           └─────────────┘       └────────────────────┘
```

1. Client → Order Service: `POST /api/v1/orders {user_id, amount}`
2. Order Service → Billing Service: `POST /api/v1/billing/charge {user_id, amount}`
3. Order Service → Notification Service: `POST /api/v1/notifications {user_id, type, message}`
4. Результат:
   - `201 Created` + `status: "paid"` — успех
   - `402 Payment Required` + `status: "failed"` — недостаточно средств

## Архитектура

Каждый сервис построен по принципам **Hexagonal Architecture**:

```
┌─────────────────────────────────────────────────────────┐
│                      Application                        │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │
│  │   Domain    │  │    Ports    │  │   Application   │  │
│  │   Models    │  │ Input/Output│  │    Services     │  │
│  └─────────────┘  └─────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────┤
│                       Adapters                          │
│  ┌─────────────────┐        ┌─────────────────────────┐ │
│  │ Primary         │        │ Secondary               │ │
│  │ (HTTP Handlers) │        │ (Repositories, Clients) │ │
│  └─────────────────┘        └─────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### Технологии

- C++17
- Boost.Beast (HTTP server/client)
- Boost.DI (Dependency Injection)
- libpqxx 7.9.2 (PostgreSQL)
- nlohmann/json
- Docker + Kubernetes

## Структура проекта

```
hw07-events/
├── CMakeLists.txt                 # Родительский CMake
├── README.md
├── billing-service/
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   ├── config.json
│   ├── include/
│   │   ├── BillingApp.hpp
│   │   ├── domain/
│   │   │   └── BillingAccount.hpp
│   │   ├── ports/
│   │   │   ├── input/IBillingService.hpp
│   │   │   └── output/
│   │   │       ├── IBillingRepository.hpp
│   │   │       └── IDbSettings.hpp
│   │   ├── application/
│   │   │   └── BillingService.hpp
│   │   └── adapters/
│   │       ├── primary/
│   │       │   ├── HealthHandler.hpp
│   │       │   └── BillingHandler.hpp
│   │       └── secondary/
│   │           ├── PostgresDbSettings.hpp
│   │           └── PostgresBillingRepository.hpp
│   └── src/
│       ├── main.cpp
│       └── BillingApp.cpp
├── notification-service/
│   └── ... (аналогичная структура)
├── order-service/
│   └── ... (+ HttpBillingClient, HttpNotificationClient)
├── k8s/
│   ├── namespace.yaml
│   ├── secrets.yaml
│   ├── billing-postgres.yaml
│   ├── billing.yaml
│   ├── notification-postgres.yaml
│   ├── notification.yaml
│   ├── order-postgres.yaml
│   ├── order.yaml
│   └── ingress.yaml
└── postman/
    └── hw07-events.postman_collection.json
```

## Сборка

### Локальная сборка

```bash
# Из корня hw07-events
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Docker образы

```bash
# Billing Service
cd billing-service
docker build -t tobantal/hw07-billing:latest .

# Notification Service
cd ../notification-service
docker build -t tobantal/hw07-notification:latest .

# Order Service
cd ../order-service
docker build -t tobantal/hw07-order:latest .
```

### Push в Docker Hub

```bash
docker push tobantal/hw07-billing:latest
docker push tobantal/hw07-notification:latest
docker push tobantal/hw07-order:latest
```

## Деплой в Kubernetes

### 1. Создание namespace и secrets

```bash
kubectl apply -f k8s/namespace.yaml
kubectl apply -f k8s/secrets.yaml
```

### 2. Деплой PostgreSQL

```bash
kubectl apply -f k8s/billing-postgres.yaml
kubectl apply -f k8s/notification-postgres.yaml
kubectl apply -f k8s/order-postgres.yaml

# Проверка
kubectl get pods -n hw07
```

### 3. Деплой сервисов

```bash
kubectl apply -f k8s/billing.yaml
kubectl apply -f k8s/notification.yaml
kubectl apply -f k8s/order.yaml

# Проверка
kubectl get pods -n hw07
kubectl get svc -n hw07
```

### 4. Ingress

```bash
# Если есть конфликт с предыдущими ДЗ - удалить старый ingress
kubectl get ingress -A
kubectl delete ingress hw06-ingress -n hw06  # если существует

# Применить новый
kubectl apply -f k8s/ingress.yaml

# Проверка
kubectl get ingress -n hw07
```

### 5. Проверка логов

```bash
kubectl logs -f deployment/billing-service -n hw07
kubectl logs -f deployment/notification-service -n hw07
kubectl logs -f deployment/order-service -n hw07
```

### 6. Health checks

```bash
curl http://arch.homework/otusapp/tobolkin/orders/health
curl http://arch.homework/otusapp/tobolkin/billing/health
curl http://arch.homework/otusapp/tobolkin/notification/health
```

## API Endpoints

Все эндпоинты доступны через единый prefix: `/otusapp/tobolkin/`

### Order Service (/otusapp/tobolkin/orders)

| Method | Path | Description |
|--------|------|-------------|
| GET | /health | Health check |
| POST | /api/v1/orders | Создать заказ |
| GET | /api/v1/orders/{id} | Получить заказ |

### Billing Service (/otusapp/tobolkin/billing)

| Method | Path | Description |
|--------|------|-------------|
| GET | /health | Health check |
| POST | /api/v1/billing/accounts | Создать аккаунт |
| POST | /api/v1/billing/deposit | Пополнить баланс |
| POST | /api/v1/billing/charge | Списать средства |
| GET | /api/v1/billing/accounts/{userId} | Получить аккаунт |

### Notification Service (/otusapp/tobolkin/notification)

| Method | Path | Description |
|--------|------|-------------|
| GET | /health | Health check |
| POST | /api/v1/notifications | Создать уведомление |
| GET | /api/v1/notifications?userId=X | Получить уведомления |

## Тестирование

### Postman

Импортировать коллекцию `postman/hw07-events.postman_collection.json`.

```bash
# Запустить тесты 
newman run postman/hw07-events.postman_collection.json 

# С подробным выводом 
newman run postman/hw07-events.postman_collection.json --reporters cli,json
```

**Сценарий тестирования (8 шагов):**

1. **Create Billing Account** — создать аккаунт с балансом 0
2. **Deposit 1000** — пополнить баланс на 1000
3. **Create Order SUCCESS (500)** — успешный заказ на 500
4. **Check Balance (500)** — баланс должен быть 500
5. **Check Notification SUCCESS** — уведомление `order.success`
6. **Create Order FAIL (600)** — неуспешный заказ (недостаточно средств)
7. **Check Balance (still 500)** — баланс не изменился
8. **Check Notification FAIL** — уведомление `order.failed`

#### Результаты postman тестов
```bash
HW07 Event-Driven Architecture

→ 1. Create Billing Account
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 213B, 49ms]
  ✓  Status code is 201
  ✓  Account created

→ 2. Deposit 1000
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 214B, 11ms]
  ✓  Status code is 200
  ✓  Balance is 1000

→ 3. Create Order (SUCCESS - 500)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [201 Created, 282B, 14ms]
  ✓  Status code is 201
  ✓  Order status is paid

→ 4. Check Balance (should be 500)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/user-rtmnhs553 [200 OK, 182B, 6ms]
  ✓  Status code is 200
  ✓  Balance is 500

→ 5. Check Notification (SUCCESS)
  GET http://arch.homework/otusapp/tobolkin/notification/api/v1/notifications?userId=user-rtmnhs553 [200 OK, 369B, 7ms]
  ✓  Status code is 200
  ✓  Has success notification

→ 6. Create Order (FAIL - 600, insufficient funds)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [402 Payment Required, 332B, 9ms]
  ✓  Status code is 402 (Payment Required)
  ✓  Order status is failed
  ✓  Error is Insufficient funds

→ 7. Check Balance (still 500)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/user-rtmnhs553 [200 OK, 182B, 4ms]
  ✓  Status code is 200
  ✓  Balance is still 500

→ 8. Check Notification (FAIL)
  GET http://arch.homework/otusapp/tobolkin/notification/api/v1/notifications?userId=user-rtmnhs553 [200 OK, 582B, 6ms]
  ✓  Status code is 200
  ✓  Has 2 notifications (success + failed)
  ✓  Second notification is failed

┌─────────────────────────┬──────────────────┬──────────────────┐
│                         │         executed │           failed │
├─────────────────────────┼──────────────────┼──────────────────┤
│              iterations │                1 │                0 │
├─────────────────────────┼──────────────────┼──────────────────┤
│                requests │                8 │                0 │
├─────────────────────────┼──────────────────┼──────────────────┤
│            test-scripts │                8 │                0 │
├─────────────────────────┼──────────────────┼──────────────────┤
│      prerequest-scripts │                1 │                0 │
├─────────────────────────┼──────────────────┼──────────────────┤
│              assertions │               18 │                0 │
├─────────────────────────┴──────────────────┴──────────────────┤
│ total run duration: 326ms                                     │
├───────────────────────────────────────────────────────────────┤
│ total data received: 1.21kB (approx)                          │
├───────────────────────────────────────────────────────────────┤
│ average response time: 13ms [min: 4ms, max: 49ms, s.d.: 13ms] │
└───────────────────────────────────────────────────────────────┘
```

### curl примеры

```bash
# Базовый URL (через Ingress)
BASE_URL="http://arch.homework/otusapp/tobolkin"

# 1. Создать billing account
curl -X POST "$BASE_URL/billing/api/v1/billing/accounts" \
  -H "Content-Type: application/json" \
  -d '{"user_id": "user-123"}'

# 2. Пополнить баланс
curl -X POST "$BASE_URL/billing/api/v1/billing/deposit" \
  -H "Content-Type: application/json" \
  -d '{"user_id": "user-123", "amount": 1000}'

# 3. Создать заказ (успех)
curl -X POST "$BASE_URL/orders/api/v1/orders" \
  -H "Content-Type: application/json" \
  -d '{"user_id": "user-123", "amount": 500}'

# 4. Проверить баланс
curl "$BASE_URL/billing/api/v1/billing/accounts/user-123"

# 5. Проверить уведомления
curl "$BASE_URL/notification/api/v1/notifications?userId=user-123"

# 6. Создать заказ (недостаточно средств)
curl -X POST "$BASE_URL/orders/api/v1/orders" \
  -H "Content-Type: application/json" \
  -d '{"user_id": "user-123", "amount": 600}'
```

## Конфигурация

### Environment Variables

**Billing Service:**
- `DB_HOST` — хост PostgreSQL
- `DB_PORT` — порт PostgreSQL (5432)
- `DB_NAME` — имя БД (billing)
- `DB_USER` — пользователь
- `DB_PASSWORD` — пароль (через K8s Secret)

**Notification Service:**
- `DB_HOST`, `DB_PORT`, `DB_NAME`, `DB_USER`, `DB_PASSWORD`

**Order Service:**
- `DB_HOST`, `DB_PORT`, `DB_NAME`, `DB_USER`, `DB_PASSWORD`
- `BILLING_HOST` — хост Billing Service
- `BILLING_PORT` — порт Billing Service (8001)
- `NOTIFICATION_HOST` — хост Notification Service
- `NOTIFICATION_PORT` — порт Notification Service (8002)

## Схемы БД

### billing.billing_accounts

```sql
CREATE TABLE billing_accounts (
    user_id VARCHAR(255) PRIMARY KEY,
    balance BIGINT NOT NULL DEFAULT 0
);
```

### notification.notifications

```sql
CREATE TABLE notifications (
    id VARCHAR(255) PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    type VARCHAR(100) NOT NULL,
    message TEXT NOT NULL,
    created_at VARCHAR(50) NOT NULL
);
CREATE INDEX idx_notifications_user_id ON notifications(user_id);
```

### orders.orders

```sql
CREATE TABLE orders (
    id VARCHAR(255) PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    amount BIGINT NOT NULL,
    status VARCHAR(50) NOT NULL,
    created_at VARCHAR(50) NOT NULL
);
CREATE INDEX idx_orders_user_id ON orders(user_id);
```

## Troubleshooting

### Pods не запускаются

```bash
# Проверить события
kubectl describe pod <pod-name> -n hw07

# Проверить логи
kubectl logs <pod-name> -n hw07
```

### Connection refused к PostgreSQL

```bash
# Проверить, что PostgreSQL pod готов
kubectl get pods -n hw07 | grep postgres

# Проверить сервис
kubectl get svc -n hw07 | grep postgres
```

### Order Service не может достучаться до Billing/Notification

```bash
# Проверить DNS resolution внутри пода
kubectl exec -it deployment/order-service -n hw07 -- \
  nslookup billing-service

# Проверить connectivity
kubectl exec -it deployment/order-service -n hw07 -- \
  curl http://billing-service:8001/health
```

## Автор

Anton Tobolkin — OTUS Microservice Architecture Course
