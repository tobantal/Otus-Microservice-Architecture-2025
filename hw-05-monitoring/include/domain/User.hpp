#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace domain {

/**
 * @brief Модель пользователя для CRUD операций
 * 
 * Соответствует спецификации OTUS:
 * https://app.swaggerhub.com/apis/otus55/users/1.0.0
 */
struct User {
    int64_t id = 0;
    std::string username;
    std::string firstName;
    std::string lastName;
    std::string email;
    std::string phone;

    /**
     * @brief Сериализация в JSON
     */
    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"username", username},
            {"firstName", firstName},
            {"lastName", lastName},
            {"email", email},
            {"phone", phone}
        };
    }

    /**
     * @brief Десериализация из JSON
     */
    static User fromJson(const nlohmann::json& j) {
        User user;
        
        if (j.contains("id") && !j["id"].is_null()) {
            user.id = j["id"].get<int64_t>();
        }
        if (j.contains("username")) {
            user.username = j["username"].get<std::string>();
        }
        if (j.contains("firstName")) {
            user.firstName = j["firstName"].get<std::string>();
        }
        if (j.contains("lastName")) {
            user.lastName = j["lastName"].get<std::string>();
        }
        if (j.contains("email")) {
            user.email = j["email"].get<std::string>();
        }
        if (j.contains("phone")) {
            user.phone = j["phone"].get<std::string>();
        }
        
        return user;
    }
};

} // namespace domain
