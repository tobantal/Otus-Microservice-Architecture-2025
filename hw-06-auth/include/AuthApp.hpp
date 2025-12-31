#pragma once

#include <BoostBeastApplication.hpp>

/**
 * @brief Приложение аутентификации для HW06
 * 
 * Наследует BoostBeastApplication с Template Method паттерном:
 * 1. loadEnvironment() - загрузка config.json
 * 2. configureInjection() - настройка Boost.DI и регистрация handlers
 * 3. start() - запуск HTTP сервера (из базового класса)
 * 
 * Архитектура: Hexagonal (Ports & Adapters)
 * - Primary Adapters: HTTP Handlers
 * - Secondary Adapters: PostgresDb, FakeJwt
 * 
 * Dependency Injection: Boost.DI
 * - Биндинги интерфейсов к реализациям
 * - Автоматическое разрешение зависимостей
 * - Singleton scope для stateful адаптеров
 * 
 * Endpoints:
 * - GET  /health              - Health check
 * - POST /api/v1/auth/register - Регистрация
 * - POST /api/v1/auth/login    - Вход
 * - POST /api/v1/auth/logout   - Выход
 * - GET  /api/v1/profile       - Получить профиль
 * - PUT  /api/v1/profile       - Обновить профиль
 */
class AuthApp : public BoostBeastApplication {
public:
    AuthApp();
    ~AuthApp() override;

protected:
    /**
     * @brief Загрузить конфигурацию
     */
    void loadEnvironment(int argc, char* argv[]) override;

    /**
     * @brief Настроить Boost.DI контейнер и зарегистрировать handlers
     * 
     * Использует Boost.DI для:
     * 1. Биндинга Output Ports (интерфейсов) к Secondary Adapters (реализациям)
     * 2. Биндинга Input Ports к Application Services
     * 3. Создания Primary Adapters (Handlers) с автоматическим разрешением зависимостей
     */
    void configureInjection() override;
};
