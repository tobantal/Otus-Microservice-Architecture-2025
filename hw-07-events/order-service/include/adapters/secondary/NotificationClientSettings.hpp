#pragma once

#include "ports/output/IServiceSettings.hpp"
#include <cstdlib>
#include <stdexcept>
#include <iostream>

namespace order::adapters::secondary {

/**
 * @brief Настройки Notification Service из переменных окружения
 * 
 * Env variables:
 * - NOTIFICATION_HOST (обязательная)
 * - NOTIFICATION_PORT (обязательная)
 */
class NotificationClientSettings : public ports::output::IServiceSettings {
public:
    NotificationClientSettings() {
        std::cout << "[NotificationClientSettings] Reading from environment" << std::endl;
    }

    std::string getHost() const override {
        const char* host = std::getenv("NOTIFICATION_HOST");
        if (!host || std::string(host).empty()) {
            throw std::runtime_error("NOTIFICATION_HOST environment variable is required");
        }
        return host;
    }

    int getPort() const override {
        const char* port = std::getenv("NOTIFICATION_PORT");
        if (!port || std::string(port).empty()) {
            throw std::runtime_error("NOTIFICATION_PORT environment variable is required");
        }
        return std::stoi(port);
    }
};

} // namespace order::adapters::secondary
