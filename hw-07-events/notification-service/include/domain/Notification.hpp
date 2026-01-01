#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>

namespace notification::domain {

/**
 * @brief Уведомление пользователя
 */
struct Notification {
    std::string id;
    std::string userId;
    std::string type;     // "order.success", "order.failed"
    std::string message;
    std::string createdAt;

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"user_id", userId},
            {"type", type},
            {"message", message},
            {"created_at", createdAt}
        };
    }

    static Notification fromJson(const nlohmann::json& j) {
        Notification n;
        n.id = j.value("id", "");
        n.userId = j.value("user_id", "");
        n.type = j.value("type", "");
        n.message = j.value("message", "");
        n.createdAt = j.value("created_at", "");
        return n;
    }
};

} // namespace notification::domain
