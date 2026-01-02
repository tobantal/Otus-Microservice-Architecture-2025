# HW09: Idempotency Pattern

## Описание

Расширение HW08 (Choreography Saga) с добавлением **идемпотентности** для операции создания заказа.

## Проблема

Без идемпотентности повторный запрос создаёт дубликат:

```
Client                          Order Service                    Billing
   │ POST /orders                      │                            │
   │──────────────────────────────────▶│ создаёт заказ              │
   │                                   │───────────────────────────▶│ списывает деньги
   │         ⚡ TIMEOUT ⚡               │                            │
   │                                   │                            │
   │ POST /orders (retry)              │                            │
   │──────────────────────────────────▶│ создаёт ВТОРОЙ заказ!      │
   │                                   │───────────────────────────▶│ списывает ДВАЖДЫ!
```

## Решение: Idempotency Key

```
Client                          Order Service
   │ POST /orders                      │
   │ X-Idempotency-Key: abc-123        │
   │──────────────────────────────────▶│ создаёт заказ, сохраняет результат
   │         ⚡ TIMEOUT ⚡               │
   │                                   │
   │ POST /orders (retry)              │
   │ X-Idempotency-Key: abc-123        │
   │──────────────────────────────────▶│ находит сохранённый результат
   │     200 OK (тот же order_id)      │
   │◀──────────────────────────────────│ возвращает его, БЕЗ повторного создания
```

## Архитектура

### Декоратор IdempotentHandler

```cpp
class IdempotentHandler : public IHttpHandler {
    IdempotentHandler(shared_ptr<IHttpHandler> inner,
                      shared_ptr<IIdempotencyRepository> repo);
    
    void handle(IRequest& req, IResponse& res) override {
        auto key = req.getHeader("X-Idempotency-Key");
        
        if (req.getMethod() == "POST" && !key.empty()) {
            // 1. Проверяем кэш
            auto cached = repo_->find(key);
            if (cached) {
                res.setStatus(cached->status);
                res.setBody(cached->body);
                return;  // Возвращаем сохранённый результат
            }
            
            // 2. Выполняем оригинальный хэндлер
            inner_->handle(req, res);
            
            // 3. Сохраняем результат
            repo_->save(key, res.getStatus(), res.getBody());
        } else {
            inner_->handle(req, res);
        }
    }
};
```

### Хранилище: PostgreSQL

```sql
CREATE TABLE idempotency_keys (
    key VARCHAR(64) PRIMARY KEY,
    response_status INTEGER,
    response_body TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);
```

## Choreography Saga (из HW08)

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

## API Endpoints

Base URL: `http://arch.homework/otusapp/tobolkin`

| Service | Path | Method | Endpoint | Description |
|---------|------|--------|----------|-------------|
| **Order** | /orders | POST | `/api/v1/orders` | Создать заказ (идемпотентный) |
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

### Idempotency Header

```bash
# Первый запрос
curl -X POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders \
  -H "Content-Type: application/json" \
  -H "X-Idempotency-Key: unique-key-123" \
  -d '{"user_id": "user1", "product_id": "prod1", "amount": 100, "quantity": 1}'

# Response: {"order_id": "order-123", "status": "BILLING_PENDING"}

# Повторный запрос с тем же ключом
curl -X POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders \
  -H "Content-Type: application/json" \
  -H "X-Idempotency-Key: unique-key-123" \
  -d '{"user_id": "user1", "product_id": "prod1", "amount": 100, "quantity": 1}'

# Response: {"order_id": "order-123", "status": "BILLING_PENDING"}
# Header: X-Idempotency-Key-Used: true
# (тот же order_id, заказ НЕ дублируется)
```

## Структура проекта

```
hw09-idempotency/
├── idempotent-order-service/
│   ├── include/
│   │   ├── adapters/
│   │   │   ├── primary/
│   │   │   │   ├── OrderHandler.hpp
│   │   │   │   ├── IdempotentHandler.hpp    # NEW: Декоратор
│   │   │   │   └── HealthHandler.hpp
│   │   │   └── secondary/
│   │   │       ├── PostgresOrderRepository.hpp
│   │   │       ├── PostgresIdempotencyRepository.hpp  # NEW
│   │   │       └── RabbitMQAdapter.hpp
│   │   ├── ports/
│   │   │   ├── input/
│   │   │   │   └── IOrderService.hpp
│   │   │   └── output/
│   │   │       ├── IOrderRepository.hpp
│   │   │       ├── IIdempotencyRepository.hpp  # NEW
│   │   │       ├── IEventPublisher.hpp
│   │   │       └── IEventConsumer.hpp
│   │   └── domain/
│   │       ├── Order.hpp
│   │       └── IdempotencyRecord.hpp  # NEW
│   └── src/
│       ├── main.cpp
│       └── OrderApp.cpp
├── idempotent-billing-service/
├── idempotent-warehouse-service/
├── idempotent-delivery-service/
├── k8s/
└── postman/
    └── hw09-idempotency.postman_collection.json
```

## Docker Images

```bash
# Build
docker build -t tobantal/hw09-idempotent-order:latest ./idempotent-order-service
docker build -t tobantal/hw09-idempotent-billing:latest ./idempotent-billing-service
docker build -t tobantal/hw09-idempotent-warehouse:latest ./idempotent-warehouse-service
docker build -t tobantal/hw09-idempotent-delivery:latest ./idempotent-delivery-service

# Push
docker push tobantal/hw09-idempotent-order:latest
docker push tobantal/hw09-idempotent-billing:latest
docker push tobantal/hw09-idempotent-warehouse:latest
docker push tobantal/hw09-idempotent-delivery:latest
```

## Развёртывание

```bash
# 1. Namespace
kubectl create namespace hw09

# 2. Инфраструктура

# вначале RabbitMQ стартуем
kubectl apply -f k8s/rabbitmq.yaml -n hw09

kubectl apply -f k8s/order-postgres.yaml -n hw09
kubectl apply -f k8s/billing-postgres.yaml -n hw09
kubectl apply -f k8s/warehouse-postgres.yaml -n hw09
kubectl apply -f k8s/delivery-postgres.yaml -n hw09

kubectl apply -f k8s/secret.yaml -n hw09

# 3. Сервисы
kubectl apply -f k8s/order-service.yaml -n hw09
kubectl apply -f k8s/billing-service.yaml -n hw09
kubectl apply -f k8s/warehouse-service.yaml -n hw09
kubectl apply -f k8s/delivery-service.yaml -n hw09

# 4. Ingress
kubectl apply -f k8s/ingress.yaml -n hw09
```

## Тестирование

```bash
newman run postman/hw09-idempotency.postman_collection.json
```

### Тесты идемпотентности (секция 6)

| # | Тест | Проверка |
|---|------|----------|
| 6.5 | First Order Request | Первый запрос создаёт заказ |
| 6.7 | Retry Order Request | Повторный запрос возвращает тот же order_id |
| 6.8 | Verify Balance | Баланс списан только один раз |
| 6.9 | Third Retry | Третий запрос тоже возвращает тот же order_id |
| 6.10 | Different Key | Другой ключ создаёт новый заказ |
| 6.11 | Verify Balance | Баланс списан дважды (два разных заказа) |

#### Результаты тестов

```bash
HW09 Idempotency Tests

❏ 0. Health Checks
↳ Order Service Health
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 197B, 17ms]
  ✓  Order service is healthy

↳ Billing Service Health
  GET http://arch.homework/otusapp/tobolkin/billing/health [200 OK, 199B, 3ms]
  ✓  Billing service is healthy

↳ Warehouse Service Health
  GET http://arch.homework/otusapp/tobolkin/warehouse/health [200 OK, 201B, 3ms]
  ✓  Warehouse service is healthy

↳ Delivery Service Health
  GET http://arch.homework/otusapp/tobolkin/delivery/health [200 OK, 200B, 2ms]
  ✓  Delivery service is healthy

❏ 1. Happy Path - Full Saga Success
↳ 1.1 Create Billing Account
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 18ms]
  ✓  Account created

↳ 1.2 Deposit Funds (1000)
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 156B, 18ms]
  ✓  Deposit successful

↳ 1.3 Check Balance Before Order
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/user-1767320646997 [200 OK, 187B, 10ms]
  ✓  Balance is 1000

↳ 1.4 Add Product to Warehouse
  POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products [201 Created, 165B, 10ms]
  ✓  Product created

↳ 1.5 Check Stock Before Order
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/prod-1767320647082 [200 OK, 214B, 10ms]
  ✓  Stock is 10, reserved is 0

↳ 1.6 Add Delivery Slot
  POST http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots [201 Created, 179B, 12ms]
  ✓  Slot created

↳ 1.7 Create Order (Starts Saga)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 209B, 17ms]
  ✓  Order accepted

↳ 1.8 Wait for Saga (2 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 197B, 3ms]
  ✓  Waited for processing

↳ 1.9 Check Order Status (COMPLETED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767320647154-8 [200 OK, 221B, 10ms]
  ✓  Saga completed successfully

↳ 1.10 Verify Balance Charged (500)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/user-1767320646997 [200 OK, 186B, 8ms]
  ✓  Balance decreased by 500

↳ 1.11 Verify Stock Reserved (2)
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/prod-1767320647082 [200 OK, 213B, 9ms]
  ✓  Stock reserved

❏ 2. Billing Failed - Insufficient Funds
↳ 2.1 Create Account (No Deposit)
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 12ms]
  ✓  Account created

↳ 2.2 Create Order (Should Fail)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 209B, 20ms]
  ✓  Order accepted for processing

↳ 2.3 Wait for Saga (2 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 197B, 4ms]

↳ 2.4 Check Order Status (FAILED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767320649294-9 [200 OK, 223B, 10ms]
  ✓  Saga failed at billing step

↳ 2.5 Verify Balance Unchanged (0)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/poor-user-1767320649263 [200 OK, 189B, 8ms]
  ✓  Balance still 0

❏ 3. Warehouse Failed - Product Not Found
↳ 3.1 Create Account with Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 11ms]
  ✓  Account created

↳ 3.2 Deposit Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 156B, 17ms]
  ✓  Deposit successful

↳ 3.3 Create Order for Non-Existent Product
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 210B, 19ms]
  ✓  Order accepted

↳ 3.4 Wait for Saga (3 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 197B, 2ms]

↳ 3.5 Check Order Status (FAILED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767320651435-10 [200 OK, 229B, 11ms]
  ✓  Saga failed at warehouse step

↳ 3.6 Verify Balance Refunded (1000)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/warehouse-test-1767320651380 [200 OK, 197B, 8ms]
  ✓  Balance refunded to 1000

❏ 4. Delivery Failed - No Slots Available
↳ 4.0 Delete All Slots (cleanup)
  DELETE http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots [200 OK, 160B, 11ms]

↳ 4.1 Create Account with Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 12ms]
  ✓  Account created

↳ 4.2 Deposit Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 156B, 20ms]

↳ 4.3 Add Product (No Delivery Slots!)
  POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products [201 Created, 165B, 10ms]
  ✓  Product created

↳ 4.4 Create Order (No Slots Available)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 210B, 18ms]
  ✓  Order accepted

↳ 4.5 Wait for Saga (3 sec)
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 197B, 3ms]

↳ 4.6 Check Order Status (FAILED)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767320654615-11 [200 OK, 228B, 8ms]
  ✓  Saga failed at delivery step

↳ 4.7 Verify Balance Refunded
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/delivery-test-1767320654538 [200 OK, 196B, 10ms]
  ✓  Balance refunded to 1000

↳ 4.8 Verify Stock Released
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/delivery-prod-1767320654538 [200 OK, 223B, 8ms]
  ✓  Stock released back

❏ 5. Event Flow Verification
↳ 5.1 Setup - Full Test Environment
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 12ms]

↳ 5.2 Deposit
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 155B, 17ms]

↳ 5.3 Add Product
  POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products [201 Created, 165B, 10ms]

↳ 5.4 Add Delivery Slot
  POST http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots [201 Created, 179B, 12ms]

↳ 5.5 Create Order - Triggers Event Chain
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 210B, 17ms]
  ┌
  │ 'Event chain started: order.created'
  └
  ✓  Order triggers event: order.created

↳ 5.6 Poll Order Status (expect events processed)
  GET http://arch.homework/otusapp/tobolkin/orders/api/v1/orders/order-1767320657808-12 [200 OK, 228B, 13ms]
  ┌
  │ 'Event chain completed successfully'
  └
  ✓  All events processed: billing.charged → warehouse.reserved → delivery.booked

↳ 5.7 Verify Event: billing.charged
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/event-user-1767320657720 [200 OK, 192B, 9ms]
  ✓  Event billing.charged processed: balance decreased

↳ 5.8 Verify Event: warehouse.reserved
  GET http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/stock/event-prod-1767320657720 [200 OK, 218B, 9ms]
  ✓  Event warehouse.reserved processed: stock reserved

❏ 6. Idempotency Tests
↳ 6.1 Setup - Create Account
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts [201 Created, 165B, 10ms]
  ✓  Account created

↳ 6.2 Deposit Funds
  POST http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/deposit [200 OK, 156B, 15ms]

↳ 6.3 Add Product
  POST http://arch.homework/otusapp/tobolkin/warehouse/api/v1/warehouse/products [201 Created, 165B, 11ms]

↳ 6.4 Add Delivery Slot
  POST http://arch.homework/otusapp/tobolkin/delivery/api/v1/delivery/slots [201 Created, 179B, 11ms]

↳ 6.5 First Order Request (with Idempotency Key)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 210B, 29ms]
  ✓  First request: Order accepted
  ✓  No X-Idempotency-Key-Used header on first request

↳ 6.6 Wait for Saga
  GET http://arch.homework/otusapp/tobolkin/orders/health [200 OK, 197B, 3ms]

↳ 6.7 Retry Order Request (same Idempotency Key)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 240B, 9ms]
  ✓  Retry returns same order_id
  ✓  X-Idempotency-Key-Used header present
  ✓  Idempotent response: no duplicate order

↳ 6.8 Verify Balance (charged only once)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/idempotency-user-1767320660908 [200 OK, 198B, 8ms]
  ✓  Balance charged only once (900)

↳ 6.9 Third Retry (same key)
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 240B, 9ms]
  ✓  Third retry still returns same order_id
  ✓  X-Idempotency-Key-Used header present

↳ 6.10 Different Key Creates New Order
  POST http://arch.homework/otusapp/tobolkin/orders/api/v1/orders [202 Accepted, 210B, 33ms]
  ✓  Different key creates new order
  ✓  No X-Idempotency-Key-Used header (new order)

↳ 6.11 Verify Balance (charged twice now)
  GET http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts/idempotency-user-1767320660908 [200 OK, 198B, 9ms]
  ✓  Balance charged twice (800)

┌─────────────────────────┬──────────────────┬─────────────────┐
│                         │         executed │          failed │
├─────────────────────────┼──────────────────┼─────────────────┤
│              iterations │                1 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│                requests │               54 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│            test-scripts │               41 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│      prerequest-scripts │               14 │               0 │
├─────────────────────────┼──────────────────┼─────────────────┤
│              assertions │               46 │               0 │
├─────────────────────────┴──────────────────┴─────────────────┤
│ total run duration: 17.2s                                    │
├──────────────────────────────────────────────────────────────┤
│ total data received: 2.66kB (approx)                         │
├──────────────────────────────────────────────────────────────┤
│ average response time: 11ms [min: 2ms, max: 33ms, s.d.: 6ms] │
└──────────────────────────────────────────────────────────────┘

```
