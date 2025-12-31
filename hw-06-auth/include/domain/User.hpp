#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace domain {

/**
 * @brief Пользователь системы
 * 
 * Содержит учётные данные для аутентификации.
 * Профиль хранится отдельно в Profile.
 */
struct User {
    std::string id;              ///< UUID пользователя
    std::string username;        ///< Уникальный логин
    std::string passwordHash;    ///< Хеш пароля (bcrypt/sha256)
    std::string createdAt;       ///< Дата создания ISO8601

    User() = default;

    User(const std::string& id,
         const std::string& username,
         const std::string& passwordHash)
        : id(id)
        , username(username)
        , passwordHash(passwordHash)
    {}

    /**
     * @brief Сериализация в JSON (без passwordHash!)
     */
    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"username", username},
            {"created_at", createdAt}
        };
    }
};

} // namespace domain
