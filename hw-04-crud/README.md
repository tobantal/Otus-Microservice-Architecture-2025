# ДЗ-04: Инфраструктурные паттерны (CRUD + PostgreSQL)

> **Студент:** Anton Tobolkin  
> **Docker образ:** `tobantal/otus-hw04:v1`  
> **Хост:** `arch.homework`

---

## 📁 Структура проекта

```
hw-04-crud/
├── CMakeLists.txt
├── Dockerfile
├── config.json
├── include/
│   ├── domain/
│   │   └── User.hpp
│   ├── repository/
│   │   ├── IUserRepository.hpp
│   │   └── PostgresUserRepository.hpp
│   ├── handlers/
│   │   ├── HealthCheckHandler.hpp
│   │   └── UserHandler.hpp
│   └── UserCrudApp.hpp
├── src/
│   ├── main.cpp
│   ├── UserCrudApp.cpp
│   ├── repository/
│   │   └── PostgresUserRepository.cpp
│   └── handlers/
│       ├── HealthCheckHandler.cpp
│       └── UserHandler.cpp
├── sql/
│   └── init.sql
├── k8s/
│   ├── postgres.yaml        # PostgreSQL (Deployment + Service + PVC)
│   ├── configmap.yaml       # DB_HOST, DB_PORT, DB_NAME
│   ├── secret.yaml          # DB_USER, DB_PASSWORD
│   ├── migration-job.yaml   # Job для SQL миграций
│   ├── deployment.yaml      # 2 реплики, probes
│   ├── service.yaml         # ClusterIP
│   └── ingress.yaml         # arch.homework
├── helm/
│   └── values-postgres.yaml
├── postman/
│   └── hw04-users.postman_collection.json
└── README.md
```

---

## 🚀 Инструкция по запуску

### Предварительные требования

- Minikube запущен
- nginx-ingress установлен (из ДЗ-03)
- minikube tunnel запущен
- arch.homework прописан в /etc/hosts

### Шаг 0. Сборка и публикация Docker образа

```bash
cd hw-04-crud

# Собрать образ (на M1 добавить --platform linux/amd64)
docker build --platform linux/amd64 -t tobantal/otus-hw04:v1 .

# Залогиниться в DockerHub
docker login

# Запушить образ
docker push tobantal/otus-hw04:v1

# ВАЖНО: Сделать образ публичным на DockerHub!
# https://hub.docker.com → Repository → Settings → Make public
```

### Шаг 1. Установка PostgreSQL

```bash
cd hw-04-crud/k8s

# Установить PostgreSQL (манифест, без Helm)
kubectl apply -f postgres.yaml

# Ожидание готовности (1-2 минуты)
kubectl wait --for=condition=ready pod \
  --selector=app=postgres \
  --timeout=120s

# Проверить статус
kubectl get pods -l app=postgres
```

> **Альтернатива через Helm (OCI registry):**
> ```bash
> helm install postgres oci://registry-1.docker.io/bitnamicharts/postgresql \
>   -f helm/values-postgres.yaml
> ```

### Шаг 2. Применение ConfigMap и Secret

```bash
cd hw-04-crud/k8s

kubectl apply -f configmap.yaml
kubectl apply -f secret.yaml
```

### Шаг 3. Выполнение миграций

```bash
kubectl apply -f migration-job.yaml

# Дождаться завершения
kubectl wait --for=condition=complete job/usercrud-migration --timeout=60s

# Проверить логи миграции
kubectl logs job/usercrud-migration
```

### Шаг 4. Деплой приложения

```bash
kubectl apply -f deployment.yaml
kubectl apply -f service.yaml
kubectl apply -f ingress.yaml

# Дождаться готовности
kubectl get pods -l app=usercrud -w
```

### Шаг 5. Проверка

```bash
# Health check
curl http://arch.homework/health

# Создать пользователя
curl -X POST http://arch.homework/api/v1/users \
  -H "Content-Type: application/json" \
  -d '{"username":"test","firstName":"Test","lastName":"User","email":"test@example.com","phone":"+123"}'

# Получить всех пользователей
curl http://arch.homework/api/v1/users
```

---

## 🧪 Тестирование через Newman

```bash
cd hw-04-crud/postman
newman run hw04-users.postman_collection.json
```

**Тесты:**
1. Health Check
2. Create User → сохраняет ID
3. Get User by ID
4. Update User
5. Get Updated User
6. Delete User
7. Get Deleted User (404)
8. Get All Users

---

## 🧹 Удаление

```bash
cd hw-04-crud/k8s

# Удалить приложение
kubectl delete -f deployment.yaml
kubectl delete -f service.yaml
kubectl delete -f ingress.yaml

# Удалить миграцию
kubectl delete job usercrud-migration
kubectl delete configmap usercrud-sql

# Удалить ConfigMap и Secret приложения
kubectl delete -f configmap.yaml
kubectl delete -f secret.yaml

# Удалить PostgreSQL
kubectl delete -f postgres.yaml
```

---

## 📋 API Endpoints

| Method | Path | Описание |
|--------|------|----------|
| GET | /health | Health check |
| POST | /api/v1/users | Создать пользователя |
| GET | /api/v1/users | Список всех пользователей |
| GET | /api/v1/users/{id} | Получить по ID |
| PUT | /api/v1/users/{id} | Обновить |
| DELETE | /api/v1/users/{id} | Удалить |

### User Model

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

---

## ✅ Чеклист выполнения

| Требование | Статус |
|------------|--------|
| RESTful CRUD API | ✅ |
| PostgreSQL для хранения | ✅ |
| ConfigMap для конфигурации | ✅ |
| Secret для credentials | ✅ |
| Job для миграций | ✅ |
| Ingress на arch.homework | ✅ |
| Postman коллекция | ✅ |
| Newman тесты | ✅ |

---

## 📎 Ссылки

- **DockerHub:** https://hub.docker.com/r/tobantal/otus-hw04
- **Swagger API:** https://app.swaggerhub.com/apis/otus55/users/1.0.0
