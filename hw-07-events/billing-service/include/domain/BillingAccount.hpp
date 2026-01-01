#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace billing::domain {

/**
 * @brief Биллинговый аккаунт пользователя
 */
struct BillingAccount {
    std::string userId;
    int64_t balance{0};  // В копейках/центах для точности

    BillingAccount() = default;
    explicit BillingAccount(const std::string& userId) : userId(userId), balance(0) {}

    nlohmann::json toJson() const {
        return {
            {"user_id", userId},
            {"balance", balance}
        };
    }

    static BillingAccount fromJson(const nlohmann::json& j) {
        BillingAccount acc;
        acc.userId = j.value("user_id", "");
        acc.balance = j.value("balance", int64_t{0});
        return acc;
    }
};

} // namespace billing::domain
