#pragma once

#include <string>
#include <cstdlib>
#include <stdexcept>

namespace billing::settings {

class RabbitMQSettings {
public:
    RabbitMQSettings() {
        host_ = getEnvOrDefault("RABBITMQ_HOST", "rabbitmq");
        port_ = std::stoi(getEnvOrDefault("RABBITMQ_PORT", "5672"));
        user_ = getEnvOrDefault("RABBITMQ_USER", "saga");
        password_ = getRequiredEnv("RABBITMQ_PASSWORD");
    }
    
    std::string getHost() const { return host_; }
    int getPort() const { return port_; }
    std::string getUser() const { return user_; }
    std::string getPassword() const { return password_; }

private:
    std::string host_;
    int port_;
    std::string user_;
    std::string password_;
    
    static std::string getRequiredEnv(const char* name) {
        const char* value = std::getenv(name);
        if (!value) throw std::runtime_error(std::string("Required env: ") + name);
        return std::string(value);
    }
    
    static std::string getEnvOrDefault(const char* name, const char* def) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : std::string(def);
    }
};

} // namespace billing::settings
