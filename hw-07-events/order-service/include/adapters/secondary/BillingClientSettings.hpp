#pragma once

#include "ports/output/IServiceSettings.hpp"
#include <cstdlib>
#include <stdexcept>
#include <iostream>

namespace order::adapters::secondary {

/**
 * @brief Настройки Billing Service из переменных окружения
 * 
 * Env variables:
 * - BILLING_HOST (обязательная)
 * - BILLING_PORT (обязательная)
 */
class BillingClientSettings : public ports::output::IServiceSettings {
public:
    BillingClientSettings() {
        std::cout << "[BillingClientSettings] Reading from environment" << std::endl;
    }

    std::string getHost() const override {
        const char* host = std::getenv("BILLING_HOST");
        if (!host || std::string(host).empty()) {
            throw std::runtime_error("BILLING_HOST environment variable is required");
        }
        return host;
    }

    int getPort() const override {
        const char* port = std::getenv("BILLING_PORT");
        if (!port || std::string(port).empty()) {
            throw std::runtime_error("BILLING_PORT environment variable is required");
        }
        return std::stoi(port);
    }
};

} // namespace order::adapters::secondary
