#include "UserCrudApp.hpp"
#include "handlers/HealthCheckHandler.hpp"
#include "handlers/UserHandler.hpp"
#include "repository/PostgresUserRepository.hpp"
#include <boost/di.hpp>
#include <iostream>
#include <cstdlib>

namespace di = boost::di;

UserCrudApp::UserCrudApp()
    : dbHost_("localhost")
    , dbPort_(5432)
    , dbName_("users_db")
    , dbUser_("postgres")
    , dbPassword_("postgres")
{
    std::cout << "[UserCrudApp] Application created" << std::endl;
}

UserCrudApp::~UserCrudApp() {
    std::cout << "[UserCrudApp] Application destroyed" << std::endl;
}

void UserCrudApp::loadEnvironment(int argc, char* argv[]) {
    std::cout << "[UserCrudApp] Loading environment..." << std::endl;
    
    // Загружаем базовый config.json
    BoostBeastApplication::loadEnvironment(argc, argv);
    
    // Читаем переменные окружения для БД (из ConfigMap/Secret)
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
    
    std::cout << "[UserCrudApp] DB Config: " 
              << dbHost_ << ":" << dbPort_ << "/" << dbName_ 
              << " (user: " << dbUser_ << ")" << std::endl;
    
    std::cout << "[UserCrudApp] Environment loaded successfully" << std::endl;
}

void UserCrudApp::configureInjection() {
    std::cout << "[UserCrudApp] Configuring dependency injection..." << std::endl;

    // Создаём PostgreSQL репозиторий
    try {
        userRepository_ = std::make_shared<repository::PostgresUserRepository>(
            dbHost_, dbPort_, dbName_, dbUser_, dbPassword_
        );
    } catch (const std::exception& e) {
        std::cerr << "[UserCrudApp] Failed to connect to database: " << e.what() << std::endl;
        throw;
    }

    // Создаём DI injector
    auto injector = di::make_injector(
        di::bind<IEnvironment>().to(env_)
    );

    // Регистрируем Health handler
    {
        auto handler = std::make_shared<HealthCheckHandler>();
        handlers_[getHandlerKey("GET", "/health")] = handler;
        std::cout << "[UserCrudApp] Registered: GET /health" << std::endl;
    }

    // Регистрируем User handler для всех методов
    {
        auto handler = std::make_shared<UserHandler>(userRepository_);
        
        // POST /api/v1/users - создание
        handlers_[getHandlerKey("POST", "/api/v1/users")] = handler;
        std::cout << "[UserCrudApp] Registered: POST /api/v1/users" << std::endl;
        
        // GET /api/v1/users - список всех
        handlers_[getHandlerKey("GET", "/api/v1/users")] = handler;
        std::cout << "[UserCrudApp] Registered: GET /api/v1/users" << std::endl;
        
        // Для операций с конкретным пользователем по ID
        // Регистрируем тот же handler - он сам разберёт path
        handlers_[getHandlerKey("GET", "/api/v1/users/*")] = handler;
        handlers_[getHandlerKey("PUT", "/api/v1/users/*")] = handler;
        handlers_[getHandlerKey("DELETE", "/api/v1/users/*")] = handler;
        std::cout << "[UserCrudApp] Registered: GET/PUT/DELETE /api/v1/users/{id}" << std::endl;
    }

    std::cout << "[UserCrudApp] DI configuration completed" << std::endl;
}
