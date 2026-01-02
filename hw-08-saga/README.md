# HW08: Choreography Saga Pattern

## Архитектура

**Choreography Saga** - каждый сервис самостоятельно слушает и публикует события в RabbitMQ.
Нет центрального оркестратора - сервисы реагируют на события друг друга.

```
                         RabbitMQ (saga.events exchange)
                                    │
     ┌──────────────────────────────┼──────────────────────────────┐
     │                              │                              │
     ▼                              ▼                              ▼
┌─────────────┐  order.created  ┌─────────────┐ billing.charged ┌─────────────┐
│   Order     │ ───────────────►│   Billing   │ ───────────────►│  Warehouse  │
│   Service   │                 │   Service   │                 │   Service   │
│  :8000      │                 │  :8001      │                 │  :8002      │
└─────────────┘                 └─────────────┘                 └─────────────┘
     ▲                               │                               │
     │                               │ warehouse.failed              │ warehouse.reserved
     │                               ▼                               ▼
     │                          ┌─────────┐                    ┌─────────────┐
     │◄─────────────────────────│ refund  │                    │  Delivery   │
     │   billing.refunded       └─────────┘                    │   Service   │
     │                                                         │  :8003      │
     │◄────────────────────────────────────────────────────────┴─────────────┘
                    delivery.booked / delivery.failed
```

## Event Flow

### События и подписки

| Сервис | Публикует | Слушает |
|--------|-----------|---------|
| **Order** | `order.created` | `billing.*`, `warehouse.*`, `delivery.*` |
| **Billing** | `billing.charged`, `billing.failed`, `billing.refunded` | `order.created`, `warehouse.failed`, `delivery.failed` |
| **Warehouse** | `warehouse.reserved`, `warehouse.failed`, `warehouse.released` | `billing.charged`, `delivery.failed` |
| **Delivery** | `delivery.booked`, `delivery.failed` | `warehouse.reserved` |

### Happy Path
```
order.created → billing.charged → warehouse.reserved → delivery.booked → COMPLETED
```

### Compensation (Rollback)

**Delivery Failed:**
```
delivery.failed → warehouse.released → billing.refunded → FAILED
```

**Warehouse Failed:**
```
warehouse.failed → billing.refunded → FAILED
```

**Billing Failed:**
```
billing.failed → FAILED (ничего не откатывать)
```

## API Endpoints

Base URL: `http://arch.homework/otusapp/tobolkin`

| Service | Path | Method | Endpoint | Description |
|---------|------|--------|----------|-------------|
| **Order** | /orders | POST | `/api/v1/orders` | Создать заказ (запускает Saga) |
| | | GET | `/api/v1/orders/{id}` | Получить статус заказа |
| | | GET | `/health` | Health check |
| **Billing** | /billing | POST | `/api/v1/billing/accounts` | Создать аккаунт |
| | | POST | `/api/v1/billing/deposit` | Пополнить баланс |
| | | GET | `/api/v1/billing/accounts/{userId}` | Получить баланс |
| | | GET | `/health` | Health check |
| **Warehouse** | /warehouse | POST | `/api/v1/warehouse/products` | Добавить товар |
| | | GET | `/api/v1/warehouse/stock/{productId}` | Получить остатки |
| | | GET | `/health` | Health check |
| **Delivery** | /delivery | POST | `/api/v1/delivery/slots` | Добавить слот доставки |
| | | DELETE | `/api/v1/delivery/slots` | Удалить все слоты (для тестов) |
| | | GET | `/health` | Health check |

## Установка

### 0. Подготовка окружения

```bash
# Запустить minikube
minikube start

# Добавить arch.homework в /etc/hosts (если ещё не добавлен)
grep -q "arch.homework" /etc/hosts || sudo sh -c 'echo "127.0.0.1 arch.homework" >> /etc/hosts'

# Запустить minikube tunnel (в отдельном терминале, оставить работать)
minikube tunnel
```

### 1. Build и Push Docker Images в DockerHub

```bash
cd hw08-saga

# Order Service
cd saga-order-service
docker build -t tobantal/hw08-saga-order:latest .
docker push tobantal/hw08-saga-order:latest
cd ..

# Billing Service
cd saga-billing-service
docker build -t tobantal/hw08-saga-billing:latest .
docker push tobantal/hw08-saga-billing:latest
cd ..

# Warehouse Service
cd saga-warehouse-service
docker build -t tobantal/hw08-saga-warehouse:latest .
docker push tobantal/hw08-saga-warehouse:latest
cd ..

# Delivery Service
cd saga-delivery-service
docker build -t tobantal/hw08-saga-delivery:latest .
docker push tobantal/hw08-saga-delivery:latest
cd ..
```

### 2. Deploy to Kubernetes

```bash
# Namespace and secrets
kubectl apply -f k8s/namespace.yaml
kubectl apply -f k8s/secret.yaml

# RabbitMQ
kubectl apply -f k8s/rabbitmq.yaml
kubectl wait --for=condition=ready pod -l app=rabbitmq -n hw08 --timeout=120s

# Databases
kubectl apply -f k8s/order-postgres.yaml
kubectl apply -f k8s/billing-postgres.yaml
kubectl apply -f k8s/warehouse-postgres.yaml
kubectl apply -f k8s/delivery-postgres.yaml

# Wait for databases
kubectl wait --for=condition=ready pod -l app=order-postgres -n hw08 --timeout=120s
kubectl wait --for=condition=ready pod -l app=billing-postgres -n hw08 --timeout=120s
kubectl wait --for=condition=ready pod -l app=warehouse-postgres -n hw08 --timeout=120s
kubectl wait --for=condition=ready pod -l app=delivery-postgres -n hw08 --timeout=120s

# Services
kubectl apply -f k8s/order-service.yaml
kubectl apply -f k8s/billing-service.yaml
kubectl apply -f k8s/warehouse-service.yaml
kubectl apply -f k8s/delivery-service.yaml

# Ingress
kubectl apply -f k8s/ingress.yaml
```

### 3. Verify Deployment

```bash
kubectl get pods -n hw08
kubectl get ingress -n hw08

# Проверить health endpoints
curl http://arch.homework/otusapp/tobolkin/orders/health
curl http://arch.homework/otusapp/tobolkin/billing/health
curl http://arch.homework/otusapp/tobolkin/warehouse/health
curl http://arch.homework/otusapp/tobolkin/delivery/health
```

## Тестирование

### Postman Tests

Коллекция `postman/hw08-saga-choreography.postman_collection.json` содержит полные тесты всех сценариев.

#### Запуск через Postman GUI

1. Импортировать коллекцию: `File → Import → hw08-saga-choreography.postman_collection.json`
2. Настроить переменные окружения (если нужно изменить хосты)
3. Запустить: `Runner → Select Collection → Run`

#### Запуск через Newman (CLI)

```bash
# Установка Newman
npm install -g newman

# Запуск всех тестов
newman run postman/hw08-saga-choreography.postman_collection.json

# С отчётом
newman run postman/hw08-saga-choreography.postman_collection.json \
  --reporters cli,html \
  --reporter-html-export report.html

# Только определённую папку
newman run postman/hw08-saga-choreography.postman_collection.json \
  --folder "1. Happy Path - Full Saga Success"
```

### Тестовые сценарии

| # | Сценарий | Ожидаемый результат |
|---|----------|---------------------|
| 0 | Health Checks | Все 4 сервиса healthy |
| 1 | **Happy Path** | `COMPLETED`, баланс уменьшен, товар зарезервирован |
| 2 | **Billing Failed** | `FAILED`, баланс не изменён |
| 3 | **Warehouse Failed** | `FAILED`, баланс возвращён (refund) |
| 4 | **Delivery Failed** | `FAILED`, товар освобождён, баланс возвращён |
| 5 | **Event Flow** | Проверка цепочки событий |

#### Результаты тестов

```bash
HW08 Choreography Saga Tests

❏ 0. Health Checks
↳ Order Service Health
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 191B, 17ms]
  ✓  Order service is healthy

↳ Billing Service Health
  GET http://arch.homework/otusapp/tobolkin/billing/health [200 OK, 193B, 3ms]
  ✓  Billing service is healthy

↳ Warehouse Service Health
  GET http://arch.homework/otusapp/tobolkin/warehouse/health [200 OK, 195B, 2ms]
  ✓  Warehouse service is healthy

↳ Delivery Service Health
  GET http://arch.homework/otusapp/tobolkin/delivery/health [200 OK, 194B, 3ms]
  ✓  Delivery service is healthy

❏ 1. Happy Path - Full Saga Success
↳ 1.1 Create Billing Account
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 12ms]
  ✓  Account created

↳ 1.2 Deposit Funds (1000)
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 156B, 18ms]
  ✓  Deposit successful

↳ 1.3 Check Balance Before Order
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/user-1767313972620 [200 OK, 187B, 9ms]
  ✓  Balance is 1000

↳ 1.4 Add Product to Warehouse
  POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products [201 Created, 165B, 10ms]
  ✓  Product created

↳ 1.5 Check Stock Before Order
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/prod-1767313972700 [200 OK, 214B, 9ms]
  ✓  Stock is 10, reserved is 0

↳ 1.6 Add Delivery Slot
  POST http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots [201 Created, 179B, 10ms]
  ✓  Slot created

↳ 1.7 Create Order (Starts Saga)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 209B, 20ms]
  ✓  Order accepted

↳ 1.8 Wait for Saga (2 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 191B, 4ms]
  ✓  Waited for saga processing

↳ 1.9 Check Order Status (COMPLETED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767313972772-7 [200 OK, 221B, 10ms]
  ✓  Saga completed successfully

↳ 1.10 Verify Balance Charged (500)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/user-1767313972620 [200 OK, 186B, 9ms]
  ✓  Balance decreased by 500

↳ 1.11 Verify Stock Reserved (2)
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/prod-1767313972700 [200 OK, 213B, 10ms]
  ✓  Stock reserved

❏ 2. Billing Failed - Insufficient Funds
↳ 2.1 Create Account (No Deposit)
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 13ms]
  ✓  Account created

↳ 2.2 Create Order (Should Fail)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 209B, 18ms]
  ✓  Order accepted for processing

↳ 2.3 Wait for Saga (2 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 191B, 4ms]

↳ 2.4 Check Order Status (FAILED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767313974915-8 [200 OK, 223B, 11ms]
  ✓  Saga failed at billing step

↳ 2.5 Verify Balance Unchanged (0)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/poor-user-1767313974884 [200 OK, 189B, 10ms]
  ✓  Balance still 0

❏ 3. Warehouse Failed - Product Not Found
↳ 3.1 Create Account with Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 11ms]
  ✓  Account created

↳ 3.2 Deposit Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 156B, 17ms]
  ✓  Deposit successful

↳ 3.3 Create Order for Non-Existent Product
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 209B, 17ms]
  ✓  Order accepted

↳ 3.4 Wait for Saga (3 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 191B, 5ms]

↳ 3.5 Check Order Status (FAILED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767313977056-9 [200 OK, 228B, 10ms]
  ✓  Saga failed at warehouse step

↳ 3.6 Verify Balance Refunded (1000)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/warehouse-test-1767313977002 [200 OK, 197B, 11ms]
  ✓  Balance refunded to 1000

❏ 4. Delivery Failed - No Slots Available
↳ 4.0 Delete All Slots (cleanup)
  DELETE http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots [200 OK, 160B, 9ms]

↳ 4.1 Create Account with Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 11ms]
  ✓  Account created

↳ 4.2 Deposit Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 156B, 17ms]

↳ 4.3 Add Product (No Delivery Slots!)
  POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products [201 Created, 165B, 11ms]
  ✓  Product created

↳ 4.4 Create Order (No Slots Available)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 210B, 19ms]
  ✓  Order accepted

↳ 4.5 Wait for Saga (3 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 191B, 3ms]

↳ 4.6 Check Order Status (FAILED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767313980235-10 [200 OK, 228B, 10ms]
  ✓  Saga failed at delivery step

↳ 4.7 Verify Balance Refunded
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/delivery-test-1767313980161 [200 OK, 196B, 8ms]
  ✓  Balance refunded to 1000

↳ 4.8 Verify Stock Released
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/delivery-prod-1767313980161 [200 OK, 223B, 10ms]
  ✓  Stock released back

❏ 5. Event Flow Verification
↳ 5.1 Setup - Full Test Environment
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 13ms]

↳ 5.2 Deposit
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 155B, 18ms]

↳ 5.3 Add Product
  POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products [201 Created, 165B, 10ms]

↳ 5.4 Add Delivery Slot
  POST http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots [201 Created, 179B, 11ms]

↳ 5.5 Create Order - Triggers Event Chain
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 210B, 17ms]
  ┌
  │ 'Event chain started: order.created'
  └
  ✓  Order triggers event: order.created

↳ 5.6 Poll Order Status (expect events processed)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767313983430-11 [200 OK, 228B, 11ms]
  ┌
  │ 'Event chain completed successfully'
  └
  ✓  All events processed: billing.charged → warehouse.reserved → delivery.booked

↳ 5.7 Verify Event: billing.charged
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/event-user-1767313983342 [200 OK, 192B, 9ms]
  ✓  Event billing.charged processed: balance decreased

↳ 5.8 Verify Event: warehouse.reserved
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/event-prod-1767313983342 [200 OK, 218B, 9ms]
  ✓  Event warehouse.reserved processed: stock reserved

┌─────────────────────────┬──────────────────┬─────────────────┐
│                         │         executed │          failed │
├─────────────────────────┼──────────────────┼─────────────────┤
│              iterations │                1 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│                requests │               43 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│            test-scripts │               34 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│      prerequest-scripts │               11 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│              assertions │               34 │               0 │
├─────────────────────────┴──────────────────┴─────────────────┤
│ total run duration: 13.9s                                    │
├──────────────────────────────────────────────────────────────┤
│ total data received: 2.09kB (approx)                         │
├──────────────────────────────────────────────────────────────┤
│ average response time: 10ms [min: 2ms, max: 20ms, s.d.: 4ms] │
└──────────────────────────────────────────────────────────────┘
```

### Ручное тестирование

```bash
# 1. Создать аккаунт
curl -X POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts \
  -H "Content-Type: application/json" \
  -d '{"user_id": "user1"}'

# 2. Пополнить баланс
curl -X POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit \
  -H "Content-Type: application/json" \
  -d '{"user_id": "user1", "amount": 1000}'

# 3. Добавить товар
curl -X POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products \
  -H "Content-Type: application/json" \
  -d '{"product_id": "prod1", "quantity": 10}'

# 4. Добавить слот доставки
curl -X POST http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots \
  -H "Content-Type: application/json" \
  -d '{"slot_time": "2025-01-15 10:00"}'

# 5. Создать заказ (запускает Choreography Saga)
curl -X POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders \
  -H "Content-Type: application/json" \
  -d '{"user_id": "user1", "product_id": "prod1", "amount": 500, "quantity": 2}'

# Response: {"order_id": "order-xxx", "status": "BILLING_PENDING"}

# 6. Проверить статус (после 2-3 секунд)
curl http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-xxx

# Response: {"id": "order-xxx", "state": "COMPLETED", ...}

# 7. Проверить баланс (должен быть 500)
curl http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/user1

# 8. Проверить резерв (должен быть reserved=2)
curl http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/prod1
```

## Docker Images

| Image | DockerHub |
|-------|-----------|
| saga-order-service | `tobantal/hw08-saga-order:latest` |
| saga-billing-service | `tobantal/hw08-saga-billing:latest` |
| saga-warehouse-service | `tobantal/hw08-saga-warehouse:latest` |
| saga-delivery-service | `tobantal/hw08-saga-delivery:latest` |

## Ingress Routes

| Path | Service |
|------|---------|
| `/otusapp/tobolkin/orders/*` | saga-order-service:8000 |
| `/otusapp/tobolkin/billing/*` | saga-billing-service:8001 |
| `/otusapp/tobolkin/warehouse/*` | saga-warehouse-service:8002 |
| `/otusapp/tobolkin/delivery/*` | saga-delivery-service:8003 |

## Принципы реализации

1. **Hexagonal Architecture** - порты и адаптеры, чистое разделение слоёв
2. **Boost.DI encapsulation** - `#include <boost/di.hpp>` только в `*App.cpp`
3. **pimpl для RabbitMQ** - AMQP-CPP/boost скрыты в `.cpp` файлах
4. **Event-Driven** - все коммуникации через RabbitMQ события
5. **Compensation** - автоматический откат при ошибках на любом шаге
