# ДЗ-03: Основы работы с Kubernetes

> **Студент:** Anton Tobolkin  
> **Docker образ:** `tobantal/otus-hw02:v1`  
> **Хост:** `arch.homework`

---

## 📁 Структура проекта

```
hw-03-kubernetes/
├── k8s/
│   ├── deployment.yaml      # Deployment (2 реплики, liveness/readiness probes)
│   ├── service.yaml         # ClusterIP Service (80 → 8000)
│   └── ingress.yaml         # Ingress для arch.homework
├── postman/
│   └── hw03-health.postman_collection.json
├── README.md                # Этот файл (инструкция)
└── README_KUBERNETES.md     # Теория по Kubernetes
```

---

## 🚀 Инструкция по запуску

### Шаг 1. Запуск Minikube

```bash
minikube start --driver=docker
minikube status
```

### Шаг 2. Установка nginx-ingress

```bash
kubectl create namespace m
helm repo add ingress-nginx https://kubernetes.github.io/ingress-nginx/
helm repo update
helm install nginx ingress-nginx/ingress-nginx --namespace m

# Ожидание готовности (до 2 минут)
kubectl wait --namespace m \
  --for=condition=ready pod \
  --selector=app.kubernetes.io/component=controller \
  --timeout=120s
```

### Шаг 3. Запуск tunnel

```bash
# В ОТДЕЛЬНОМ терминале (держать открытым!)
minikube tunnel
```

### Шаг 4. Настройка /etc/hosts

```bash
# Получить IP nginx-ingress LoadBalancer
NGINX_IP=$(kubectl get svc -n m nginx-ingress-nginx-controller \
  -o jsonpath='{.status.loadBalancer.ingress[0].ip}')

echo "Nginx IP: $NGINX_IP"

# Добавить в /etc/hosts
echo "$NGINX_IP arch.homework" | sudo tee -a /etc/hosts
```

### Шаг 5. Деплой приложения

```bash
cd hw-03-kubernetes/k8s

# Применить все манифесты
kubectl apply -f .

# Дождаться Ready (READY = 1/1, STATUS = Running)
kubectl get pods -w
# Выход: Ctrl+C
```

### Шаг 6. Проверка

```bash
# Статус ресурсов
kubectl get deployments
kubectl get pods
kubectl get services
kubectl get ingress

# Тест endpoint
curl http://arch.homework/health
# Ожидаемый ответ: {"status":"ok"}
```

---

## 🧪 Тестирование через Newman

```bash
cd hw-03-kubernetes/postman
newman run hw03-health.postman_collection.json
```

---

## 🧹 Удаление

```bash
# Удалить приложение
cd hw-03-kubernetes/k8s
kubectl delete -f .

# Удалить nginx-ingress (опционально)
helm uninstall nginx -n m
kubectl delete namespace m

# Удалить запись из /etc/hosts (опционально)
sudo sed -i '/arch.homework/d' /etc/hosts

# Остановить minikube (опционально)
minikube stop
```

---

## 🔧 Отладка

```bash
# Логи подов
kubectl logs -l app=helloworld

# Детали пода (ошибки, события)
kubectl describe pod -l app=helloworld

# Логи ingress controller
kubectl logs -n m -l app.kubernetes.io/component=controller

# Проверить endpoints
kubectl get endpoints helloworld-svc

# Прямой запрос к поду (минуя ingress)
kubectl port-forward deployment/helloworld 8000:8000 &
curl http://localhost:8000/health
```

---

## ✅ Чеклист выполнения

| Требование | Статус |
|------------|--------|
| Сервис на порту 8000 | ✅ |
| GET /health → {"status": "OK"} | ✅ |
| Docker образ на DockerHub | ✅ `tobantal/otus-hw02:v1` |
| Deployment (2+ реплики) | ✅ |
| Liveness probe | ✅ |
| Readiness probe | ✅ |
| Service (ClusterIP) | ✅ |
| Ingress (arch.homework) | ✅ |
| ingressClassName: nginx | ✅ |
| Манифесты в одной директории | ✅ |
| Postman коллекция | ✅ |

---

## 📎 Ссылки

- **DockerHub:** https://hub.docker.com/r/tobantal/otus-hw02
- **Теория:** [README_KUBERNETES.md](./README_KUBERNETES.md)
