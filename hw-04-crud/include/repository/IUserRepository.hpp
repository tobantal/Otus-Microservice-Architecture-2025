#pragma once

#include "domain/User.hpp"
#include <optional>
#include <vector>

namespace repository {

/**
 * @brief Интерфейс репозитория пользователей
 * 
 * Абстракция для работы с хранилищем пользователей.
 * Реализации: PostgresUserRepository
 */
class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    /**
     * @brief Создать пользователя
     * @return ID созданного пользователя
     */
    virtual int64_t create(const domain::User& user) = 0;

    /**
     * @brief Найти пользователя по ID
     */
    virtual std::optional<domain::User> findById(int64_t id) = 0;

    /**
     * @brief Получить всех пользователей
     */
    virtual std::vector<domain::User> findAll() = 0;

    /**
     * @brief Обновить пользователя
     * @return true если обновлён, false если не найден
     */
    virtual bool update(int64_t id, const domain::User& user) = 0;

    /**
     * @brief Удалить пользователя
     * @return true если удалён, false если не найден
     */
    virtual bool deleteById(int64_t id) = 0;
};

} // namespace repository
