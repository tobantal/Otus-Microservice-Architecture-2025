# HW06: Authentication & API Gateway

> **Курс:** OTUS Microservice Architecture  
> **Студент:** Anton Tobolkin  
> **Дата:** 2025-12-31

## 📋 Задание

Добавить в приложение аутентификацию и регистрацию пользователей.
Реализовать сценарий "Изменение и просмотр данных в профиле клиента".

**Ключевое требование:** Данные профиля для чтения и редактирования не должны быть доступны другим клиентам (аутентифицированным или нет).

## 🏗 Архитектура

```
┌─────────────────────────────────────────────────────────────────┐
│                         Client (Postman)                         │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Nginx Ingress Controller                      │
│                   arch.homework/otusapp/tobolkin                 │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Auth Service (C++17)                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                    Primary Adapters                      │    │
│  │  RegisterHandler │ LoginHandler │ ProfileHandler        │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                   Application Layer                      │    │
│  │                     AuthService                          │    │
│  │  • registerUser() • login() • logout() • validateToken() │    │
│  └─────────────────────────────────────────────────────────┘    │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │                  Secondary Adapters                      │    │
│  │  FakeJwtAdapter │ PostgresUserRepo │ PostgresProfileRepo │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                         PostgreSQL                               │
│                    users + profiles tables                       │
└─────────────────────────────────────────────────────────────────┘
```

## 🔐 Механизм защиты профиля

**Почему user2 НЕ видит профиль user1:**

1. Endpoint `/api/v1/profile` **НЕ принимает userId** в параметрах
2. userId извлекается **из JWT токена** в Authorization header
3. Токен user2 содержит userId2 → возвращается только profile2

```cpp
// ProfileHandler::handleGet()
auto token = extractBearerToken(req);        // 1. Извлечь токен
auto userId = authService_->getUserIdFromToken(*token);  // 2. userId из токена
auto profile = profileRepository_->findByUserId(*userId); // 3. Только свой профиль
```

## 📊 API Endpoints

| Method | Path | Описание | Auth |
|--------|------|----------|------|
| GET | /health | Health check | ❌ |
| POST | /api/v1/auth/register | Регистрация | ❌ |
| POST | /api/v1/auth/login | Вход | ❌ |
| POST | /api/v1/auth/logout | Выход | ✅ |
| GET | /api/v1/profile | Получить свой профиль | ✅ |
| PUT | /api/v1/profile | Обновить свой профиль | ✅ |

## 🚀 Установка

### Шаг 1. Подготовка namespace

```bash
kubectl apply -f k8s/namespace.yaml
```

### Шаг 2. Деплой PostgreSQL

```bash
kubectl apply -f k8s/configmap.yaml
kubectl apply -f k8s/secret.yaml
kubectl apply -f k8s/postgres.yaml

# Подождать готовности
kubectl wait --namespace hw06 \
    --for=condition=ready pod \
    --selector=app=hw06-postgres \
    --timeout=120s
```

### Шаг 3. Сборка и публикация Docker образа

```bash
docker build -t tobantal/hw06-auth:latest .
docker push tobantal/hw06-auth:latest
```

### Шаг 4. Деплой приложения

```bash
kubectl apply -f k8s/deployment.yaml
kubectl apply -f k8s/service.yaml
kubectl apply -f k8s/ingress.yaml

# Проверить
kubectl get pods -n hw06
```

### Шаг 5. Запуск minikube tunnel

```bash
# В отдельном терминале
minikube tunnel
```

### Шаг 6. Настройка /etc/hosts

```bash
INGRESS_IP=$(kubectl get svc -n m nginx-ingress-nginx-controller -o wide | awk 'NR==2 {print $4}')
sudo sed -i '/arch.homework/d' /etc/hosts
echo "$INGRESS_IP arch.homework" | sudo tee -a /etc/hosts
```

### Шаг 7. Проверка

```bash
curl http://arch.homework/otusapp/tobolkin/health
# {"status":"ok"}
```

## 🧪 Тестирование (Postman)

### Запуск через Newman

```bash
# Установка newman
npm install -g newman

# Запуск тестов с выводом деталей
newman run postman/hw06-auth.postman_collection.json \
    --env-var "baseUrl=http://arch.homework/otusapp/tobolkin" \
    --reporters cli \
    --reporter-cli-show-timestamps
```

### Сценарий тестов

| # | Тест | Ожидание |
|---|------|----------|
| 1 | Register User1 | 201, user_id |
| 2 | GET /profile без токена | 401 |
| 3 | Login User1 | 200, token |
| 4 | PUT /profile (изменение) | 200, обновлённые данные |
| 5 | GET /profile (проверка) | 200, данные совпадают |
| 6 | Logout User1 | 200 |
| 7 | Register User2 | 201 |
| 8 | Login User2 | 200, token |
| 9 | GET /profile (User2 видит только свой) | 200, user2_id (НЕ user1_id!) |

## 📁 Структура проекта

```
hw06-auth/
├── CMakeLists.txt
├── Dockerfile
├── README.md
├── config.json
│
├── include/
│   ├── AuthApp.hpp                    # Main application
│   ├── domain/
│   │   ├── User.hpp
│   │   └── Profile.hpp
│   ├── ports/
│   │   ├── input/IAuthService.hpp
│   │   └── output/
│   │       ├── IJwtProvider.hpp
│   │       ├── IUserRepository.hpp
│   │       └── IProfileRepository.hpp
│   ├── application/
│   │   └── AuthService.hpp
│   └── adapters/
│       ├── primary/
│       │   ├── HealthCheckHandler.hpp
│       │   ├── RegisterHandler.hpp
│       │   ├── LoginHandler.hpp
│       │   ├── LogoutHandler.hpp
│       │   └── ProfileHandler.hpp     # ⭐ Ключевой компонент
│       └── secondary/
│           ├── FakeJwtAdapter.hpp
│           ├── PostgresUserRepository.hpp
│           └── PostgresProfileRepository.hpp
│
├── src/
│   └── main.cpp
│
├── sql/
│   └── init.sql
│
├── k8s/
│   ├── namespace.yaml
│   ├── configmap.yaml
│   ├── secret.yaml
│   ├── postgres.yaml
│   ├── deployment.yaml
│   ├── service.yaml
│   └── ingress.yaml
│
└── postman/
    └── hw06-auth.postman_collection.json
```

## ✅ Checklist

- [x] Схема архитектуры
- [x] Регистрация пользователя
- [x] Вход (получение токена)
- [x] Выход (инвалидация токена)
- [x] Просмотр своего профиля
- [x] Редактирование своего профиля
- [x] Защита: без токена → 401
- [x] Защита: user2 не видит профиль user1
- [x] Postman тесты (9 шагов)
- [x] Newman запуск из командной строки
- [x] Kubernetes манифесты
- [x] Docker образ

## 🔗 Полезные команды

```bash
# Логи приложения
kubectl logs -n hw06 -l app=hw06-auth -f

# Логи PostgreSQL
kubectl logs -n hw06 -l app=hw06-postgres -f

# Подключение к PostgreSQL
kubectl exec -it -n hw06 deploy/hw06-postgres -- psql -U auth -d auth

# Проверка таблиц
kubectl exec -it -n hw06 deploy/hw06-postgres -- psql -U auth -d auth -c "SELECT * FROM users;"
kubectl exec -it -n hw06 deploy/hw06-postgres -- psql -U auth -d auth -c "SELECT * FROM profiles;"
```
