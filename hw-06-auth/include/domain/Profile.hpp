#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace domain {

/**
 * @brief Профиль пользователя
 * 
 * Содержит персональные данные, доступные только владельцу.
 */
struct Profile {
    std::string userId;          ///< ID владельца (FK на User.id)
    std::string firstName;       ///< Имя
    std::string lastName;        ///< Фамилия
    std::string email;           ///< Email
    std::string phone;           ///< Телефон
    std::string updatedAt;       ///< Дата последнего обновления

    Profile() = default;

    Profile(const std::string& userId)
        : userId(userId)
    {}

    Profile(const std::string& userId,
            const std::string& firstName,
            const std::string& lastName,
            const std::string& email,
            const std::string& phone)
        : userId(userId)
        , firstName(firstName)
        , lastName(lastName)
        , email(email)
        , phone(phone)
    {}

    /**
     * @brief Сериализация в JSON
     */
    nlohmann::json toJson() const {
        return {
            {"user_id", userId},
            {"first_name", firstName},
            {"last_name", lastName},
            {"email", email},
            {"phone", phone},
            {"updated_at", updatedAt}
        };
    }

    /**
     * @brief Десериализация из JSON
     */
    static Profile fromJson(const nlohmann::json& j, const std::string& userId) {
        Profile p(userId);
        
        if (j.contains("first_name") && j["first_name"].is_string()) {
            p.firstName = j["first_name"].get<std::string>();
        }
        if (j.contains("last_name") && j["last_name"].is_string()) {
            p.lastName = j["last_name"].get<std::string>();
        }
        if (j.contains("email") && j["email"].is_string()) {
            p.email = j["email"].get<std::string>();
        }
        if (j.contains("phone") && j["phone"].is_string()) {
            p.phone = j["phone"].get<std::string>();
        }
        
        return p;
    }
};

} // namespace domain
