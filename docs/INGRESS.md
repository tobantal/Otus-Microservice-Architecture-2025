# Kubernetes Ingress: Руководство

## Что такое Ingress?

**Ingress** — это объект Kubernetes, который управляет внешним HTTP/HTTPS доступом к сервисам внутри кластера. Работает как "умный" реверс-прокси.

### Ключевые понятия

| Компонент | Описание |
|-----------|----------|
| **Ingress Controller** | Реальный nginx (или traefik, haproxy), который обрабатывает трафик. Один на кластер. |
| **Ingress Resource** | YAML-манифест с правилами маршрутизации. Много на кластер (по одному на приложение). |
| **Host** | Доменное имя (arch.homework) |
| **Path** | URL-путь (/otusapp/tobolkin/orders, etc.) |
| **Backend** | Сервис Kubernetes, куда направляется трафик |

## Архитектура

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              ВНЕШНИЙ МИР                                    │
│                                                                             │
│   curl http://arch.homework/otusapp/tobolkin/orders/api/v1/orders           │
│                                                                             │
└───────────────────────────────────┬─────────────────────────────────────────┘
                                    │
                                    ▼
┌───────────────────────────────────────────────────────────────────────────────┐
│                          INGRESS CONTROLLER (nginx)                           │
│                                                                               │
│   Слушает на портах 80/443                                                    │
│   Читает все Ingress Resources в кластере                                     │
│   Генерирует nginx.conf из правил                                             │
│                                                                               │
└───────────────────────────────────┬───────────────────────────────────────────┘
                                    │
                                    ▼
┌───────────────────────────────────────────────────────────────────────────────┐
│                           INGRESS RESOURCE (hw07)                             │
│                                                                               │
│   host: arch.homework                                                         │
│   rules:                                                                      │
│     /otusapp/tobolkin/orders/*       → order-service:8000                     │
│     /otusapp/tobolkin/billing/*      → billing-service:8001                   │
│     /otusapp/tobolkin/notification/* → notification-service:8002              │
│                                                                               │
└───────────────────────────────────┬───────────────────────────────────────────┘
                                    │
                    ┌───────────────┼───────────────┐
                    ▼               ▼               ▼
              ┌─────────┐    ┌───────────┐   ┌──────────────┐
              │  Order  │    │  Billing  │   │ Notification │
              │ Service │    │  Service  │   │   Service    │
              │  :8000  │    │   :8001   │   │    :8002     │
              └─────────┘    └───────────┘   └──────────────┘
```

## Как работает rewrite-target

Аннотация `nginx.ingress.kubernetes.io/rewrite-target` позволяет **срезать prefix** из URL.

### Пример

```yaml
metadata:
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /$2   # $2 = второй capture group
spec:
  rules:
  - host: arch.homework
    http:
      paths:
      - path: /otusapp/tobolkin/orders(/|$)(.*)
        #      └──────────────────────┘└─┘└──┘
        #              prefix          $1  $2
```

### Преобразование URL

| Внешний запрос (клиент отправляет) | После rewrite (сервис получает) |
|------------------------------------|--------------------------------|
| `/otusapp/tobolkin/orders/health` | `/health` |
| `/otusapp/tobolkin/orders/api/v1/orders` | `/api/v1/orders` |
| `/otusapp/tobolkin/billing/api/v1/billing/accounts` | `/api/v1/billing/accounts` |
| `/otusapp/tobolkin/notification/api/v1/notifications` | `/api/v1/notifications` |

**Важно:** Сервис НЕ знает про `/otusapp/tobolkin/...` — Ingress срезает этот prefix!

---

## Частые проблемы

### Проблема: Конфликт путей

```
Error from server (BadRequest): admission webhook "validate.nginx.ingress.kubernetes.io" 
denied the request: host "arch.homework" and path "/otusapp/tobolkin(/|$)(.*)" 
is already defined in ingress hw06/hw06-ingress
```

**Причина:** Один и тот же `host + path` уже занят в другом Ingress (в другом namespace).

**Решение:** Удалить старый Ingress (см. раздел "Управление Ingress").

### Проблема: 404 Not Found

**Возможные причины:**

1. **Ingress не применён:**
   ```bash
   kubectl get ingress -n hw07
   ```

2. **Сервис не существует:**
   ```bash
   kubectl get svc -n hw07
   ```

3. **Pod не запущен:**
   ```bash
   kubectl get pods -n hw07
   ```

4. **Неправильный путь в запросе:**
   ```bash
   # Правильно:
   curl http://arch.homework/otusapp/tobolkin/orders/health
   
   # Неправильно:
   curl http://arch.homework/health
   ```

### Проблема: 502 Bad Gateway

**Причина:** Ingress Controller не может достучаться до backend сервиса.

**Диагностика:**
```bash
# Проверить, что pod работает
kubectl get pods -n hw07

# Проверить логи пода
kubectl logs -f deployment/order-service -n hw07

# Проверить, что сервис имеет endpoints
kubectl get endpoints -n hw07
```

### Проблема: 503 Service Unavailable

**Причина:** Сервис есть, но нет готовых подов (readinessProbe не проходит).

```bash
# Проверить статус подов
kubectl get pods -n hw07

# Проверить события
kubectl describe pod <pod-name> -n hw07
```

---

## Управление Ingress

### Просмотр всех Ingress в кластере

```bash
# Все ingress во всех namespace
kubectl get ingress -A

# С подробностями
kubectl get ingress -A -o wide

# В конкретном namespace
kubectl get ingress -n hw07
```

### Просмотр деталей Ingress

```bash
# Краткая информация
kubectl describe ingress hw07-ingress -n hw07

# Полный YAML
kubectl get ingress hw07-ingress -n hw07 -o yaml
```

### Создание/Обновление Ingress

```bash
# Применить (создать или обновить)
kubectl apply -f k8s/ingress.yaml

# Принудительно пересоздать
kubectl delete -f k8s/ingress.yaml
kubectl apply -f k8s/ingress.yaml
```

### Удаление Ingress

```bash
# Удалить конкретный ingress
kubectl delete ingress hw07-ingress -n hw07

# Удалить по файлу
kubectl delete -f k8s/ingress.yaml

# Удалить ВСЕ ingress в namespace (осторожно!)
kubectl delete ingress --all -n hw07
```

### Удаление старых Ingress из других namespace

```bash
# Посмотреть все ingress
kubectl get ingress -A

# Удалить ingress из hw06
kubectl delete ingress hw06-ingress -n hw06

# Или удалить весь namespace hw06 (если больше не нужен)
kubectl delete namespace hw06
```

---

## Отладка Ingress Controller

### Логи Ingress Controller

```bash
# Найти pod ingress controller
kubectl get pods -n ingress-nginx

# Логи (последние 100 строк)
kubectl logs -n ingress-nginx deployment/ingress-nginx-controller --tail=100

# Логи в реальном времени
kubectl logs -n ingress-nginx deployment/ingress-nginx-controller -f
```

### Проверка конфигурации nginx внутри контроллера

```bash
# Зайти внутрь пода
kubectl exec -it -n ingress-nginx deployment/ingress-nginx-controller -- /bin/bash

# Посмотреть сгенерированный nginx.conf
cat /etc/nginx/nginx.conf | grep -A 20 "arch.homework"
```

### Проверка connectivity изнутри кластера

```bash
# Зайти в любой под
kubectl exec -it deployment/order-service -n hw07 -- /bin/sh

# Проверить DNS
nslookup billing-service

# Проверить HTTP
curl http://billing-service:8001/health
curl http://notification-service:8002/health
```

---

## Перезапуск Ingress Controller

### Мягкий перезапуск (rolling restart)

```bash
kubectl rollout restart deployment/ingress-nginx-controller -n ingress-nginx
```

### Жёсткий перезапуск (удаление пода)

```bash
# Найти pod
kubectl get pods -n ingress-nginx

# Удалить (Kubernetes автоматически создаст новый)
kubectl delete pod ingress-nginx-controller-xxxxx -n ingress-nginx
```

### Полная переустановка Ingress Controller

```bash
# Удалить
kubectl delete namespace ingress-nginx

# Установить заново (через Helm или kubectl)
kubectl apply -f https://raw.githubusercontent.com/kubernetes/ingress-nginx/controller-v1.8.2/deploy/static/provider/cloud/deploy.yaml
```

---

## Полезные команды (шпаргалка)

```bash
# ========== ПРОСМОТР ==========
kubectl get ingress -A                              # Все ingress
kubectl get ingress -n hw07                         # Ingress в namespace
kubectl describe ingress hw07-ingress -n hw07       # Детали ingress

# ========== СОЗДАНИЕ/УДАЛЕНИЕ ==========
kubectl apply -f k8s/ingress.yaml                   # Применить
kubectl delete ingress hw07-ingress -n hw07         # Удалить
kubectl delete -f k8s/ingress.yaml                  # Удалить по файлу

# ========== ОТЛАДКА ==========
kubectl logs -n ingress-nginx deployment/ingress-nginx-controller -f  # Логи
kubectl get endpoints -n hw07                       # Проверить endpoints
kubectl describe pod <pod-name> -n hw07             # События пода

# ========== ПЕРЕЗАПУСК ==========
kubectl rollout restart deployment/ingress-nginx-controller -n ingress-nginx

# ========== ОЧИСТКА СТАРЫХ ==========
kubectl get ingress -A                              # Найти все
kubectl delete ingress <name> -n <namespace>        # Удалить старый
kubectl delete namespace <old-namespace>            # Удалить весь namespace
```

---

## Схема URL для HW07

```
http://arch.homework/otusapp/tobolkin/orders/...       → Order Service
http://arch.homework/otusapp/tobolkin/billing/...      → Billing Service  
http://arch.homework/otusapp/tobolkin/notification/... → Notification Service
```

### Примеры запросов

```bash
# Health checks
curl http://arch.homework/otusapp/tobolkin/orders/health
curl http://arch.homework/otusapp/tobolkin/billing/health
curl http://arch.homework/otusapp/tobolkin/notification/health

# API calls
curl http://arch.homework/otusapp/tobolkin/billing/api/v1/billing/accounts
curl http://arch.homework/otusapp/tobolkin/orders/api/v1/orders
curl http://arch.homework/otusapp/tobolkin/notification/api/v1/notifications?userId=user-123
```
