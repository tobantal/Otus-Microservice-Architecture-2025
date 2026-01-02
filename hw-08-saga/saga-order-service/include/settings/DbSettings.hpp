#pragma once

#include <string>
#include <cstdlib>
#include <stdexcept>

namespace order::settings {

class DbSettings {
public:
    DbSettings() {
        host_ = getRequiredEnv("DB_HOST");
        port_ = std::stoi(getRequiredEnv("DB_PORT"));
        name_ = getRequiredEnv("DB_NAME");
        user_ = getRequiredEnv("DB_USER");
        password_ = getRequiredEnv("DB_PASSWORD");
    }
    
    std::string getConnectionString() const {
        return "host=" + host_ + " port=" + std::to_string(port_) +
               " dbname=" + name_ + " user=" + user_ + " password=" + password_;
    }

private:
    std::string host_, name_, user_, password_;
    int port_;
    
    static std::string getRequiredEnv(const char* name) {
        const char* value = std::getenv(name);
        if (!value) throw std::runtime_error(std::string("Required env: ") + name);
        return std::string(value);
    }
};

} // namespace order::settings
