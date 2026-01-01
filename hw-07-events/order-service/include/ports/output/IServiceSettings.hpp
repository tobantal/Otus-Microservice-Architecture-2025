#pragma once

#include <string>

namespace order::ports::output {

/**
 * @brief Интерфейс настроек внешнего сервиса
 */
class IServiceSettings {
public:
    virtual ~IServiceSettings() = default;
    virtual std::string getHost() const = 0;
    virtual int getPort() const = 0;
};

} // namespace order::ports::output
