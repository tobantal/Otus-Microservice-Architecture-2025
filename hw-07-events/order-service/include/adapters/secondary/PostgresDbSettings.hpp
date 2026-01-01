#pragma once

#include "ports/output/IDbSettings.hpp"
#include <cstdlib>
#include <stdexcept>
#include <iostream>

namespace order::adapters::secondary {

/**
 * @brief PostgreSQL настройки из переменных окружения
 */
class PostgresDbSettings : public ports::output::IDbSettings {
public:
    PostgresDbSettings() {
        std::cout << "[PostgresDbSettings] Reading from environment" << std::endl;
    }

    std::string getHost() const override {
        const char* host = std::getenv("DB_HOST");
        if (!host || std::string(host).empty()) {
            throw std::runtime_error("DB_HOST environment variable is required");
        }
        return host;
    }

    int getPort() const override {
        const char* port = std::getenv("DB_PORT");
        if (!port || std::string(port).empty()) {
            throw std::runtime_error("DB_PORT environment variable is required");
        }
        return std::stoi(port);
    }

    std::string getDbName() const override {
        const char* dbname = std::getenv("DB_NAME");
        if (!dbname || std::string(dbname).empty()) {
            throw std::runtime_error("DB_NAME environment variable is required");
        }
        return dbname;
    }

    std::string getUser() const override {
        const char* user = std::getenv("DB_USER");
        if (!user || std::string(user).empty()) {
            throw std::runtime_error("DB_USER environment variable is required");
        }
        return user;
    }

    std::string getPassword() const override {
        const char* password = std::getenv("DB_PASSWORD");
        if (!password || std::string(password).empty()) {
            throw std::runtime_error("DB_PASSWORD environment variable is required");
        }
        return password;
    }

    std::string getConnectionString() const override {
        return "host=" + getHost() + 
               " port=" + std::to_string(getPort()) + 
               " dbname=" + getDbName() + 
               " user=" + getUser() + 
               " password=" + getPassword();
    }
};

} // namespace order::adapters::secondary
