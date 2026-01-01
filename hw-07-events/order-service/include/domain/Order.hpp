#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace order::domain {

/**
 * @brief Статус заказа
 */
enum class OrderStatus {
    CREATED,
    PAID,
    FAILED
};

inline std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::CREATED: return "created";
        case OrderStatus::PAID: return "paid";
        case OrderStatus::FAILED: return "failed";
        default: return "unknown";
    }
}

/**
 * @brief Заказ
 */
struct Order {
    std::string id;
    std::string userId;
    int64_t amount{0};
    OrderStatus status{OrderStatus::CREATED};
    std::string createdAt;

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"user_id", userId},
            {"amount", amount},
            {"status", orderStatusToString(status)},
            {"created_at", createdAt}
        };
    }
};

} // namespace order::domain
