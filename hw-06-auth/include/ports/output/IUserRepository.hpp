#pragma once

#include "domain/User.hpp"
#include <optional>
#include <string>

namespace ports::output {

/**
 * @brief Интерфейс репозитория пользователей
 */
class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    /**
     * @brief Сохранить пользователя
     */
    virtual void save(const domain::User& user) = 0;

    /**
     * @brief Найти по ID
     */
    virtual std::optional<domain::User> findById(const std::string& id) = 0;

    /**
     * @brief Найти по username
     */
    virtual std::optional<domain::User> findByUsername(const std::string& username) = 0;

    /**
     * @brief Проверить существование username
     */
    virtual bool existsByUsername(const std::string& username) = 0;
};

} // namespace ports::output
