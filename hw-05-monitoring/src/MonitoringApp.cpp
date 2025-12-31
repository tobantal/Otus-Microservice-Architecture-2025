#include "MonitoringApp.hpp"
#include "handlers/HealthCheckHandler.hpp"
#include "handlers/MetricsHandler.hpp"
#include "handlers/UserHandler.hpp"
#include "repository/PostgresUserRepository.hpp"
#include <boost/di.hpp>
#include <iostream>
#include <cstdlib>

namespace di = boost::di;

MonitoringApp::MonitoringApp()
    : dbHost_("localhost")
    , dbPort_(5432)
    , dbName_("users_db")
    , dbUser_("postgres")
    , dbPassword_("postgres")
{
    std::cout << "[MonitoringApp] Application created" << std::endl;
}

MonitoringApp::~MonitoringApp() {
    std::cout << "[MonitoringApp] Application destroyed" << std::endl;
}

void MonitoringApp::loadEnvironment(int argc, char* argv[]) {
    std::cout << "[MonitoringApp] Loading environment..." << std::endl;
    
    // Загружаем базовый config.json
    BoostBeastApplication::loadEnvironment(argc, argv);
    
    // Читаем переменные окружения для БД
    if (const char* env = std::getenv("DB_HOST")) {
        dbHost_ = env;
    }
    if (const char* env = std::getenv("DB_PORT")) {
        dbPort_ = std::stoi(env);
    }
    if (const char* env = std::getenv("DB_NAME")) {
        dbName_ = env;
    }
    if (const char* env = std::getenv("DB_USER")) {
        dbUser_ = env;
    }
    if (const char* env = std::getenv("DB_PASSWORD")) {
        dbPassword_ = env;
    }
    
    std::cout << "[MonitoringApp] DB Config: " 
              << dbHost_ << ":" << dbPort_ << "/" << dbName_ 
              << " (user: " << dbUser_ << ")" << std::endl;
    
    std::cout << "[MonitoringApp] Environment loaded successfully" << std::endl;
}

void MonitoringApp::configureInjection() {
    std::cout << "[MonitoringApp] Configuring dependency injection..." << std::endl;

    // Создаём PostgreSQL репозиторий
    try {
        userRepository_ = std::make_shared<repository::PostgresUserRepository>(
            dbHost_, dbPort_, dbName_, dbUser_, dbPassword_
        );
    } catch (const std::exception& e) {
        std::cerr << "[MonitoringApp] Failed to connect to database: " << e.what() << std::endl;
        throw;
    }

    // Создаём DI injector
    auto injector = di::make_injector(
        di::bind<IEnvironment>().to(env_)
    );

    // ===========================================
    // Health endpoint
    // ===========================================
    {
        auto handler = std::make_shared<HealthCheckHandler>();
        handlers_[getHandlerKey("GET", "/health")] = handler;
        std::cout << "[MonitoringApp] Registered: GET /health" << std::endl;
    }

    // ===========================================
    // Metrics endpoint (Prometheus)
    // ===========================================
    {
        auto handler = std::make_shared<MetricsHandler>();
        handlers_[getHandlerKey("GET", "/metrics")] = handler;
        std::cout << "[MonitoringApp] Registered: GET /metrics" << std::endl;
    }

    // ===========================================
    // User CRUD endpoints
    // ===========================================
    {
        auto handler = std::make_shared<UserHandler>(userRepository_);
        
        // POST /api/v1/users - создание
        handlers_[getHandlerKey("POST", "/api/v1/users")] = handler;
        std::cout << "[MonitoringApp] Registered: POST /api/v1/users" << std::endl;
        
        // GET /api/v1/users - список всех
        handlers_[getHandlerKey("GET", "/api/v1/users")] = handler;
        std::cout << "[MonitoringApp] Registered: GET /api/v1/users" << std::endl;
        
        // GET/PUT/DELETE /api/v1/users/{id}
        handlers_[getHandlerKey("GET", "/api/v1/users/*")] = handler;
        handlers_[getHandlerKey("PUT", "/api/v1/users/*")] = handler;
        handlers_[getHandlerKey("DELETE", "/api/v1/users/*")] = handler;
        std::cout << "[MonitoringApp] Registered: GET/PUT/DELETE /api/v1/users/{id}" << std::endl;
    }

    std::cout << "[MonitoringApp] DI configuration completed" << std::endl;
}
