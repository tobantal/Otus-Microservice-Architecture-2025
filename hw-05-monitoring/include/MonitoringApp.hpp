#pragma once

#include <BoostBeastApplication.hpp>
#include <IHttpHandler.hpp>
#include "repository/IUserRepository.hpp"
#include <memory>
#include <string>

/**
 * @class MonitoringApp
 * @brief Приложение CRUD Users с Prometheus метриками
 * 
 * Расширяет hw04 добавлением:
 * - GET /metrics - endpoint для Prometheus
 * - Автоматический сбор метрик:
 *   - http_request_duration_seconds (histogram)
 *   - http_requests_total (counter)
 *   - http_errors_total (counter)
 * 
 * Наследует BoostBeastApplication с Template Method паттерном:
 * 1. loadEnvironment() - загрузка конфигурации
 * 2. configureInjection() - регистрация handlers
 * 3. start() - запуск HTTP сервера
 */
class MonitoringApp : public BoostBeastApplication {
public:
    MonitoringApp();
    virtual ~MonitoringApp();

protected:
    void loadEnvironment(int argc, char* argv[]) override;
    void configureInjection() override;

private:
    // Database settings
    std::string dbHost_;
    int dbPort_;
    std::string dbName_;
    std::string dbUser_;
    std::string dbPassword_;
    
    // Repository
    std::shared_ptr<repository::IUserRepository> userRepository_;
};
