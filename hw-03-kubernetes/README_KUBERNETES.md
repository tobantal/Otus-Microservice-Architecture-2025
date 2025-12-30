# 🎓 Kubernetes: Теория и Концепции

> Справочный материал к ДЗ-03

---

## 1. Что такое Kubernetes?

**Kubernetes (K8s)** — платформа оркестрации контейнеров от Google (2014).

**Проблема:** Docker запускает контейнеры на одной машине. А если нужно:
- 100 контейнеров на 10 серверах?
- Автоматически перезапускать упавшие?
- Балансировать нагрузку?
- Обновлять без даунтайма?

**Решение:** Kubernetes управляет всем этим автоматически.

```
┌─────────────────────────────────────────────────────────────────┐
│                     KUBERNETES CLUSTER                           │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                    Control Plane                           │  │
│  │  ┌──────────┐ ┌────────┐ ┌───────────┐ ┌───────────────┐  │  │
│  │  │API Server│ │  etcd  │ │ Scheduler │ │  Controller   │  │  │
│  │  │          │ │  (БД)  │ │           │ │   Manager     │  │  │
│  │  └──────────┘ └────────┘ └───────────┘ └───────────────┘  │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              │                                   │
│  ┌───────────────────────────┼───────────────────────────────┐  │
│  │                     Worker Nodes                           │  │
│  │  ┌─────────────────┐     │     ┌─────────────────┐        │  │
│  │  │     Node 1      │     │     │     Node 2      │        │  │
│  │  │  ┌───┐ ┌───┐    │◄────┴────►│  ┌───┐ ┌───┐    │        │  │
│  │  │  │Pod│ │Pod│    │           │  │Pod│ │Pod│    │        │  │
│  │  │  └───┘ └───┘    │           │  └───┘ └───┘    │        │  │
│  │  │    kubelet      │           │    kubelet      │        │  │
│  │  └─────────────────┘           └─────────────────┘        │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### Компоненты Control Plane

| Компонент | Роль |
|-----------|------|
| **API Server** | Единая точка входа. Все команды kubectl идут сюда |
| **etcd** | Распределённая БД. Хранит состояние кластера |
| **Scheduler** | Решает, на какой ноде запустить под |
| **Controller Manager** | Следит, чтобы текущее состояние = желаемому |

### Компоненты Worker Node

| Компонент | Роль |
|-----------|------|
| **kubelet** | Агент на каждой ноде. Запускает контейнеры |
| **kube-proxy** | Сетевой прокси. Реализует Service |
| **Container Runtime** | Docker, containerd, CRI-O |

---

## 2. Minikube — локальный кластер

**Minikube** создаёт однонодовый кластер для разработки.

```bash
# Жизненный цикл
minikube start --driver=docker   # Создать/запустить
minikube status                  # Проверить статус
minikube stop                    # Остановить (сохраняет данные)
minikube delete                  # Удалить полностью

# Полезные команды
minikube ip                      # IP кластера
minikube ssh                     # Зайти внутрь VM
minikube dashboard               # Открыть веб-интерфейс
minikube tunnel                  # Проброс LoadBalancer
minikube addons list             # Доступные аддоны
```

### Minikube Tunnel

LoadBalancer сервисы в облаке получают внешний IP автоматически. В minikube для этого нужен `minikube tunnel`:

```
БЕЗ tunnel:
  LoadBalancer Service → EXTERNAL-IP: <pending>

С tunnel (в отдельном терминале):
  LoadBalancer Service → EXTERNAL-IP: 10.110.87.218
```

---

## 3. kubectl — CLI для Kubernetes

```bash
# ═══════════════════════════════════════════════════════════════
# CRUD операции
# ═══════════════════════════════════════════════════════════════
kubectl apply -f file.yaml       # Создать/обновить ресурс
kubectl delete -f file.yaml      # Удалить ресурс
kubectl get <resource>           # Список ресурсов
kubectl describe <resource>      # Детальная информация

# ═══════════════════════════════════════════════════════════════
# Типы ресурсов
# ═══════════════════════════════════════════════════════════════
kubectl get pods                 # Поды
kubectl get svc                  # Сервисы (сокращение)
kubectl get deploy               # Деплойменты
kubectl get ing                  # Ingress
kubectl get all                  # Всё основное

# ═══════════════════════════════════════════════════════════════
# Фильтрация и форматирование
# ═══════════════════════════════════════════════════════════════
kubectl get pods -o wide                    # Больше колонок
kubectl get pods -o yaml                    # YAML формат
kubectl get pods -l app=nginx               # По лейблу
kubectl get pods -n kube-system             # В namespace
kubectl get pods -A                         # Во всех namespaces
kubectl get pods -w                         # Watch (live updates)

# ═══════════════════════════════════════════════════════════════
# Отладка
# ═══════════════════════════════════════════════════════════════
kubectl logs <pod>                          # Логи
kubectl logs -f <pod>                       # Follow (tail -f)
kubectl logs -l app=nginx                   # По лейблу
kubectl exec -it <pod> -- /bin/bash         # Shell внутри
kubectl port-forward <pod> 8080:80          # Проброс порта
```

---

## 4. Ключевые сущности K8s

### 4.1 Pod — минимальная единица

**Pod** = один или несколько контейнеров с общими ресурсами.

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: my-pod
spec:
  containers:
    - name: app
      image: nginx:alpine
      ports:
        - containerPort: 80
```

**Важно:** Поды эфемерны! Никогда не создавай их напрямую — используй Deployment.

```
┌─────────────────────────────────────┐
│              POD                    │
│  ┌─────────────┐  ┌─────────────┐   │
│  │  Container  │  │  Container  │   │
│  │    (app)    │  │  (sidecar)  │   │
│  └─────────────┘  └─────────────┘   │
│         │                │          │
│    Общая сеть (localhost)           │
│    Общие volumes                    │
│    Общий IP адрес                   │
└─────────────────────────────────────┘
```

### 4.2 Deployment — декларативное управление

**Deployment** описывает желаемое состояние, K8s поддерживает его.

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: my-app
spec:
  replicas: 3                    # Сколько подов
  selector:
    matchLabels:
      app: my-app                # Какие поды контролировать
  template:                      # Шаблон пода
    metadata:
      labels:
        app: my-app
    spec:
      containers:
        - name: app
          image: my-image:v1
```

**Что умеет Deployment:**
- Поддерживать N реплик
- Rolling Update (обновление без даунтайма)
- Rollback (откат к предыдущей версии)
- Автоматический перезапуск упавших подов

```bash
# Масштабирование
kubectl scale deployment my-app --replicas=5

# Обновление образа
kubectl set image deployment/my-app app=my-image:v2

# История версий
kubectl rollout history deployment/my-app

# Откат
kubectl rollout undo deployment/my-app
```

### 4.3 Health Checks (Probes)

```yaml
containers:
  - name: app
    image: my-image
    
    # LIVENESS: "Жив ли контейнер?"
    # Падает → K8s перезапускает контейнер
    livenessProbe:
      httpGet:
        path: /health
        port: 8000
      initialDelaySeconds: 5    # Ждать после старта
      periodSeconds: 10         # Как часто проверять
      failureThreshold: 3       # Сколько неудач до рестарта
    
    # READINESS: "Готов принимать трафик?"
    # Падает → убирается из Service (но не рестартится)
    readinessProbe:
      httpGet:
        path: /health
        port: 8000
      initialDelaySeconds: 3
      periodSeconds: 5
```

**Разница:**

| Probe | При неудаче | Когда использовать |
|-------|-------------|-------------------|
| **Liveness** | Рестарт контейнера | Deadlock, зависание |
| **Readiness** | Убрать из балансировки | Долгий старт, временная недоступность |

### 4.4 Service — стабильный endpoint

Проблема: IP подов меняются при пересоздании.

```yaml
apiVersion: v1
kind: Service
metadata:
  name: my-service
spec:
  type: ClusterIP              # Тип сервиса
  selector:
    app: my-app                # Какие поды включить
  ports:
    - port: 80                 # Порт сервиса
      targetPort: 8000         # Порт контейнера
```

**Типы Service:**

```
┌────────────────────────────────────────────────────────────────┐
│                         ClusterIP                               │
│  Доступен только ВНУТРИ кластера                                │
│  Используется для внутренней коммуникации                       │
│                                                                  │
│  my-service.default.svc.cluster.local → 10.96.0.15             │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                          NodePort                               │
│  Открывает порт 30000-32767 на КАЖДОЙ ноде                     │
│                                                                  │
│  <любая-нода-ip>:31234 → Service → Pods                        │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                        LoadBalancer                             │
│  Создаёт внешний балансировщик (в облаке)                      │
│  В minikube требует `minikube tunnel`                          │
│                                                                  │
│  EXTERNAL-IP:80 → Service → Pods                               │
└────────────────────────────────────────────────────────────────┘
```

### 4.5 Ingress — HTTP роутинг

**Ingress** — L7 балансировщик (HTTP/HTTPS).

```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: my-ingress
spec:
  ingressClassName: nginx       # ОБЯЗАТЕЛЬНО для nginx-ingress
  rules:
    - host: api.example.com     # Виртуальный хост
      http:
        paths:
          - path: /users
            pathType: Prefix
            backend:
              service:
                name: users-svc
                port:
                  number: 80
          - path: /orders
            pathType: Prefix
            backend:
              service:
                name: orders-svc
                port:
                  number: 80
```

**Ingress Controller** — реализация (nginx, traefik, haproxy). Без него Ingress ресурсы игнорируются!

```
                   Internet
                       │
                       ▼
┌──────────────────────────────────────────────────┐
│           INGRESS CONTROLLER (nginx)              │
│  ┌──────────────────────────────────────────┐    │
│  │  /users/*  → users-svc                   │    │
│  │  /orders/* → orders-svc                  │    │
│  │  /health   → health-svc                  │    │
│  └──────────────────────────────────────────┘    │
└──────────────────────────────────────────────────┘
              │              │
              ▼              ▼
        ┌──────────┐   ┌──────────┐
        │users-svc │   │orders-svc│
        └──────────┘   └──────────┘
```

### 4.6 Namespace — изоляция

```bash
kubectl get namespaces

# Стандартные:
# default        — по умолчанию
# kube-system    — системные компоненты K8s
# kube-public    — публичные ресурсы

# Создать свой
kubectl create namespace my-project

# Работать в namespace
kubectl get pods -n my-project
kubectl apply -f app.yaml -n my-project
```

---

## 5. Helm — пакетный менеджер

**Helm Chart** = папка с шаблонами + values.yaml.

```bash
# Репозитории
helm repo add bitnami https://charts.bitnami.com/bitnami
helm repo update
helm search repo nginx

# Установка
helm install my-release bitnami/nginx
helm install my-release bitnami/nginx -f values.yaml
helm install my-release bitnami/nginx --set replicaCount=3

# Управление
helm list                       # Список релизов
helm status my-release          # Статус
helm upgrade my-release ...     # Обновить
helm rollback my-release 1      # Откат
helm uninstall my-release       # Удалить
```

---

## 6. Как работает Request Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│                         REQUEST FLOW                                 │
└─────────────────────────────────────────────────────────────────────┘

Browser: GET http://arch.homework/health
                │
                ▼
┌───────────────────────────────────────┐
│  1. DNS Resolution (/etc/hosts)       │
│     arch.homework → 10.110.87.218     │
│     (IP nginx-ingress LoadBalancer)   │
└───────────────────────────────────────┘
                │
                ▼
┌───────────────────────────────────────┐
│  2. minikube tunnel                   │
│     Пробрасывает трафик внутрь VM     │
└───────────────────────────────────────┘
                │
                ▼
┌───────────────────────────────────────┐
│  3. Ingress Controller (nginx pod)    │
│     Слушает на :80, :443              │
│     Смотрит правила Ingress           │
│     Host: arch.homework               │
│     Path: /health                     │
└───────────────────────────────────────┘
                │
                ▼
┌───────────────────────────────────────┐
│  4. Service (ClusterIP)               │
│     helloworld-svc:80                 │
│     Selector: app=helloworld          │
└───────────────────────────────────────┘
                │
                ▼
┌───────────────────────────────────────┐
│  5. kube-proxy (iptables/IPVS)        │
│     Балансировка между подами         │
│     Round-robin                       │
└───────────────────────────────────────┘
                │
        ┌───────┴───────┐
        ▼               ▼
┌─────────────┐  ┌─────────────┐
│   Pod 1     │  │   Pod 2     │
│   :8000     │  │   :8000     │
└─────────────┘  └─────────────┘
        │
        ▼
┌───────────────────────────────────────┐
│  6. Container                         │
│     GET localhost:8000/health         │
│     Response: {"status":"ok"}         │
└───────────────────────────────────────┘
```

---

## 7. Полезные паттерны

### Multi-container Pod (Sidecar)

```yaml
spec:
  containers:
    - name: app
      image: my-app
    - name: log-shipper           # Sidecar
      image: fluentd
      volumeMounts:
        - name: logs
          mountPath: /var/log
```

### Init Container

```yaml
spec:
  initContainers:                 # Выполняются ДО основных
    - name: wait-for-db
      image: busybox
      command: ['sh', '-c', 'until nc -z db 5432; do sleep 1; done']
  containers:
    - name: app
      image: my-app
```

### Resource Limits

```yaml
containers:
  - name: app
    resources:
      requests:                   # Гарантированный минимум
        cpu: "100m"               # 0.1 CPU core
        memory: "128Mi"
      limits:                     # Максимум (убьют при превышении)
        cpu: "500m"
        memory: "256Mi"
```

---

## 8. Частые ошибки

| Ошибка | Причина | Решение |
|--------|---------|---------|
| `ErrImagePull` | Образ не найден | Проверить имя, приватность на DockerHub |
| `CrashLoopBackOff` | Контейнер падает | `kubectl logs`, проверить команду |
| `Pending` | Нет ресурсов/ноды | `kubectl describe pod` |
| Ingress не работает | Нет controller | Установить nginx-ingress |
| `<pending>` EXTERNAL-IP | Нет LoadBalancer | `minikube tunnel` |

---

## 📚 Дополнительные материалы

- [Kubernetes Documentation](https://kubernetes.io/docs/)
- [Kubernetes The Hard Way](https://github.com/kelseyhightower/kubernetes-the-hard-way)
- [Minikube Docs](https://minikube.sigs.k8s.io/docs/)
- [nginx-ingress Docs](https://kubernetes.github.io/ingress-nginx/)
- [Helm Docs](https://helm.sh/docs/)
- [YAML Multiline Strings](https://yaml-multiline.info/)
