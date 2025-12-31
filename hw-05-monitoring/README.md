# HW05: Prometheus & Grafana Monitoring

> **Курс:** OTUS Microservice Architecture  
> **Студент:** Anton Tobolkin  
> **Дата:** 2025-12-31

## 📋 Задание

Инструментировать сервис метриками в формате Prometheus:

- **Latency** (response time) с квантилями p50, p95, p99, max
- **RPS** (requests per second)
- **Error Rate** - количество 500ых ответов

Метрики должны быть:
1. По API методам (из приложения)
2. В целом по сервису (из nginx-ingress-controller)

Настроить алертинг в Grafana на Error Rate и Latency.

## 🏗 Архитектура

```
┌─────────────────────────────────────────────────────────────────┐
│                        Kubernetes Cluster                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────────┐         ┌──────────────┐                     │
│   │  Prometheus  │◄────────│   Grafana    │                     │
│   │   (scrape)   │         │ (dashboards) │                     │
│   └──────┬───────┘         └──────────────┘                     │
│          │                                                       │
│          │ /metrics                                              │
│          ▼                                                       │
│   ┌──────────────┐         ┌──────────────┐                     │
│   │ HW05 Service │◄────────│  PostgreSQL  │                     │
│   │  (C++/Beast) │         │    (users)   │                     │
│   └──────┬───────┘         └──────────────┘                     │
│          │                                                       │
│          │                                                       │
│   ┌──────▼───────┐                                              │
│   │ Nginx Ingress│                                              │
│   │  Controller  │◄──── /metrics (nginx metrics)                │
│   └──────────────┘                                              │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

## 📊 Метрики

### Application Metrics (GET /metrics)

| Метрика | Тип | Описание |
|---------|-----|----------|
| `http_request_duration_seconds` | Histogram | Latency с бакетами для квантилей |
| `http_request_duration_max_seconds` | Gauge | Максимальное время ответа |
| `http_requests_total` | Counter | Общее количество запросов |
| `http_errors_total` | Counter | Количество 5xx ошибок |
| `app_uptime_seconds` | Gauge | Время работы приложения |

### Nginx Ingress Metrics

| Метрика | Тип | Описание |
|---------|-----|----------|
| `nginx_ingress_controller_request_duration_seconds` | Histogram | Latency на уровне ingress |
| `nginx_ingress_controller_requests` | Counter | Количество запросов |

## 🚀 Установка

### Шаг 1. Установка Prometheus + Grafana (СНАЧАЛА!)

```bash
# Prometheus создаёт CRD для ServiceMonitor - нужен первым
helm repo add prometheus-community https://prometheus-community.github.io/helm-charts
helm repo update
helm install prometheus prometheus-community/kube-prometheus-stack \
    --namespace monitoring \
    --create-namespace

# Проверить что поды запустились
kubectl --namespace monitoring get pods -l "release=prometheus"
```

### Шаг 2. Получение пароля Grafana

```bash
# Логин: admin
# Пароль:
kubectl --namespace monitoring get secrets prometheus-grafana \
    -o jsonpath="{.data.admin-password}" | base64 -d ; echo
```

### Шаг 3. Установка nginx-ingress с метриками

```bash
# nginx-ingress с включёнными метриками
helm repo add ingress-nginx https://kubernetes.github.io/ingress-nginx
helm repo update
helm install nginx ingress-nginx/ingress-nginx \
    --namespace m \
    --create-namespace \
    --set controller.metrics.enabled=true \
    --set controller.metrics.serviceMonitor.enabled=true

# Подождать запуска
kubectl wait --namespace m \
    --for=condition=ready pod \
    --selector=app.kubernetes.io/component=controller \
    --timeout=120s
```

### Шаг 4. Создание ServiceMonitor для nginx

**ВАЖНО!** Prometheus ищет ServiceMonitor с label `release: prometheus`. Без этого шага метрики nginx НЕ будут видны.

```bash
kubectl apply -f k8s/servicemonitor-nginx.yaml
```

### Шаг 5. Сборка и публикация Docker образа

```bash
docker build -t tobantal/hw05-monitoring:latest .
docker push tobantal/hw05-monitoring:latest
```

### Шаг 6. Деплой приложения в Kubernetes

```bash
kubectl apply -f k8s/namespace.yaml
kubectl apply -f k8s/configmap.yaml
kubectl apply -f k8s/secret.yaml
kubectl apply -f k8s/postgres.yaml
kubectl apply -f k8s/deployment.yaml
kubectl apply -f k8s/service.yaml
kubectl apply -f k8s/ingress.yaml
kubectl apply -f k8s/servicemonitor.yaml

# Проверить
kubectl get pods -n hw05
```

### Шаг 7. Применение алертов

```bash
kubectl apply -f prometheus/alert-rules.yaml

# Проверить что алерты созданы
kubectl get prometheusrule -n monitoring | grep hw05
```

### Шаг 8. Настройка /etc/hosts
```bash
# Получить IP LoadBalancer и прописать в hosts
INGRESS_IP=$(kubectl get svc -n m nginx-ingress-nginx-controller -o wide | awk 'NR==2 {print $4}')
sudo sed -i '/arch.homework/d' /etc/hosts
echo "$INGRESS_IP arch.homework" | sudo tee -a /etc/hosts
```


### Шаг 9. Запуск minikube tunnel

```bash
# В ОТДЕЛЬНОМ терминале! Держать открытым.
minikube tunnel
```

### Шаг 10. Проверка доступности

```bash
curl http://arch.homework/otusapp/tobolkin/health
# Должен вернуть: {"status":"ok"}
```

### Шаг 11. Импорт Grafana Dashboard

```bash
# Port-forward к Grafana
kubectl port-forward -n monitoring svc/prometheus-grafana 3000:80
```

1. Открыть http://localhost:3000
2. Логин: `admin`, пароль: из шага 2
3. Dashboards → Import → Upload JSON
4. Выбрать файл: `grafana/dashboards/hw05-dashboard.json`

### Шаг 12. Просмотр алертов

**В Prometheus:**
```bash
kubectl port-forward -n monitoring svc/prometheus-kube-prometheus-prometheus 9090:9090
```
Открыть: http://localhost:9090/alerts

**В Grafana:**
http://localhost:3000 → Alerting → Alert rules

## 📈 Стресс-тестирование

### Установка Apache Benchmark

```bash
sudo apt-get install -y apache2-utils
```

### Запуск стресс-теста

```bash
chmod +x stress-test.sh

# Запуск на 5 минут (по умолчанию)
./stress-test.sh

# Запуск на 10 минут
DURATION=600 ./stress-test.sh

# Запуск на 1 минуту (для быстрой проверки)
DURATION=60 ./stress-test.sh
```

### Параметры

| Параметр | По умолчанию | Описание |
|----------|--------------|----------|
| `DURATION` | 300 | Длительность теста в секундах |
| `CONCURRENCY` | 50 | Количество параллельных соединений |
| `BASE_URL` | http://arch.homework/otusapp/tobolkin | URL сервиса |

## ⚠️ Алерты

Настроены следующие алерты (см. `prometheus/alert-rules.yaml`):

| Alert | Condition | Severity |
|-------|-----------|----------|
| HighErrorRate | Error rate > 5% | Warning |
| CriticalErrorRate | Error rate > 10% | Critical |
| HighLatencyP95 | P95 > 500ms | Warning |
| CriticalLatencyP99 | P99 > 1s | Critical |
| ServiceDown | Service unavailable | Critical |
| NginxHighErrorRate | Nginx 5xx > 5% | Warning |
| NginxHighLatency | Nginx P95 > 500ms | Warning |

## 📁 Структура проекта

```
hw05-monitoring/
├── CMakeLists.txt
├── Dockerfile
├── README.md
├── config.json
├── stress-test.sh
│
├── include/
│   ├── MonitoringApp.hpp
│   ├── domain/User.hpp
│   ├── handlers/
│   │   ├── HealthCheckHandler.hpp
│   │   ├── MetricsHandler.hpp
│   │   └── UserHandler.hpp
│   ├── metrics/
│   │   └── PrometheusMetrics.hpp      # ⭐ Histogram + Registry
│   └── repository/
│       ├── IUserRepository.hpp
│       └── PostgresUserRepository.hpp
│
├── src/
│   ├── main.cpp
│   ├── MonitoringApp.cpp
│   └── repository/PostgresUserRepository.cpp
│
├── sql/init.sql
│
├── k8s/
│   ├── namespace.yaml
│   ├── configmap.yaml
│   ├── secret.yaml
│   ├── postgres.yaml
│   ├── deployment.yaml
│   ├── service.yaml
│   ├── ingress.yaml
│   ├── servicemonitor.yaml           # ServiceMonitor для приложения
│   └── servicemonitor-nginx.yaml     # ⭐ ServiceMonitor для nginx
│
├── prometheus/
│   ├── prometheus-config.yaml
│   └── alert-rules.yaml              # ⭐ PrometheusRule с алертами
│
├── grafana/
│   └── dashboards/
│       └── hw05-dashboard.json
│
└── screenshots/
```

## 🔧 Troubleshooting

### Nginx метрики не отображаются

1. Проверить что ServiceMonitor создан:
   ```bash
   kubectl get servicemonitor -n m
   ```

2. ServiceMonitor должен иметь label `release: prometheus`:
   ```bash
   kubectl get servicemonitor -n m -o yaml | grep -A2 labels
   ```

3. Применить если нет:
   ```bash
   kubectl apply -f k8s/servicemonitor-nginx.yaml
   ```

### Сервис недоступен через Ingress

1. Проверить что tunnel запущен (в отдельном терминале)
2. Проверить /etc/hosts:
   ```bash
   cat /etc/hosts | grep arch.homework
   # Должно быть: <IP> arch.homework
   ```
3. Если IP пустой или неправильный:
   ```bash
   # Посмотреть EXTERNAL-IP
   kubectl get svc -n m nginx-ingress-nginx-controller
   
   # Прописать (замените на ваш IP!)
   sudo sed -i '/arch.homework/d' /etc/hosts
   echo "10.106.63.159 arch.homework" | sudo tee -a /etc/hosts
   ```

### Grafana не открывается

```bash
# Убить старые port-forward
pkill -f "port-forward"

# Запустить заново
kubectl port-forward -n monitoring svc/prometheus-grafana 3000:80
```

### Алерты не отображаются

1. Проверить что PrometheusRule создан:
   ```bash
   kubectl get prometheusrule -n monitoring | grep hw05
   ```

2. Если нет — применить:
   ```bash
   kubectl apply -f prometheus/alert-rules.yaml
   ```

## ✅ Checklist

- [✅] Метрики приложения с разбивкой по API методам
  - [✅] Latency (p50, p95, p99, max)
  - [✅] RPS
  - [✅] Error Rate (5xx)
- [✅] Метрики nginx-ingress-controller
  - [✅] Latency
  - [✅] RPS  
  - [✅] Error Rate
- [✅] Grafana Dashboard (JSON)
- [✅] Алерты на Error Rate и Latency
- [✅] Скриншоты под нагрузкой

## 🔗 API Endpoints

| Method | Path | Description |
|--------|------|-------------|
| GET | /health | Health check |
| GET | /metrics | Prometheus metrics |
| POST | /api/v1/users | Create user |
| GET | /api/v1/users | List users |
| GET | /api/v1/users/{id} | Get user by ID |
| PUT | /api/v1/users/{id} | Update user |
| DELETE | /api/v1/users/{id} | Delete user |
