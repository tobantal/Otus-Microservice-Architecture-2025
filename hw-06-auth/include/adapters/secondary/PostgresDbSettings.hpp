#pragma once

#include "ports/output/IDbSettings.hpp"
#include <cstdlib>
#include <iostream>

namespace adapters::secondary {

/**
 * @brief PostgreSQL настройки из переменных окружения
 * 
 * Читает параметры подключения из env variables:
 * - DB_HOST (default: localhost)
 * - DB_PORT (default: 5432)
 * - DB_NAME (default: auth)
 * - DB_USER (default: auth)
 * - DB_PASSWORD (default: password)
 */
class PostgresDbSettings : public ports::output::IDbSettings {
public:
    PostgresDbSettings() {
        std::cout << "[PostgresDbSettings] Reading from environment" << std::endl;
    }

    std::string getHost() const override {
        const char* env = std::getenv("DB_HOST");
        return env ? env : "localhost";
    }

    int getPort() const override {
        const char* env = std::getenv("DB_PORT");
        return env ? std::stoi(env) : 5432;
    }

    std::string getDbName() const override {
        const char* env = std::getenv("DB_NAME");
        return env ? env : "auth";
    }

    std::string getUser() const override {
        const char* env = std::getenv("DB_USER");
        return env ? env : "auth";
    }

    std::string getPassword() const override {
        const char* env = std::getenv("DB_PASSWORD");
        return env ? env : "password";
    }

    std::string getConnectionString() const override {
        return "host=" + getHost() + 
               " port=" + std::to_string(getPort()) + 
               " dbname=" + getDbName() + 
               " user=" + getUser() + 
               " password=" + getPassword();
    }
};

} // namespace adapters::secondary
