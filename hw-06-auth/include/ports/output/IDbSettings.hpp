#pragma once

#include <string>

namespace ports::output {

/**
 * @brief Интерфейс настроек базы данных
 * 
 * Абстрагирует получение параметров подключения к БД.
 * Реализация читает из env variables.
 */
class IDbSettings {
public:
    virtual ~IDbSettings() = default;

    virtual std::string getHost() const = 0;
    virtual int getPort() const = 0;
    virtual std::string getDbName() const = 0;
    virtual std::string getUser() const = 0;
    virtual std::string getPassword() const = 0;
    
    /**
     * @brief Получить connection string для pqxx
     */
    virtual std::string getConnectionString() const = 0;
};

} // namespace ports::output
