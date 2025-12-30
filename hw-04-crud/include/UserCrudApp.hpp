#pragma once

#include <BoostBeastApplication.hpp>
#include "repository/IUserRepository.hpp"
#include <memory>

/**
 * @brief Главное приложение User CRUD
 * 
 * Наследует BoostBeastApplication с Template Method паттерном:
 * 1. loadEnvironment() - загрузка config.json + переменные окружения
 * 2. configureInjection() - регистрация handlers и repository
 * 3. start() - запуск HTTP сервера
 */
class UserCrudApp : public BoostBeastApplication {
public:
    UserCrudApp();
    ~UserCrudApp() override;

protected:
    /**
     * @brief Загрузить конфигурацию
     * 
     * Читает config.json и переменные окружения для БД:
     * - DB_HOST, DB_PORT, DB_NAME, DB_USER, DB_PASSWORD
     */
    void loadEnvironment(int argc, char* argv[]) override;

    /**
     * @brief Настроить DI контейнер
     * 
     * Регистрирует:
     * - PostgresUserRepository
     * - HealthCheckHandler → GET /health
     * - UserHandler → /api/v1/users/*
     */
    void configureInjection() override;

private:
    std::shared_ptr<repository::IUserRepository> userRepository_;
    
    // DB config from environment
    std::string dbHost_;
    int dbPort_;
    std::string dbName_;
    std::string dbUser_;
    std::string dbPassword_;
};
