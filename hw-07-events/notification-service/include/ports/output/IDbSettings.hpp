#pragma once

#include <string>

namespace notification::ports::output {

/**
 * @brief Интерфейс настроек БД
 */
class IDbSettings {
public:
    virtual ~IDbSettings() = default;

    virtual std::string getHost() const = 0;
    virtual int getPort() const = 0;
    virtual std::string getDbName() const = 0;
    virtual std::string getUser() const = 0;
    virtual std::string getPassword() const = 0;
    virtual std::string getConnectionString() const = 0;
};

} // namespace notification::ports::output
