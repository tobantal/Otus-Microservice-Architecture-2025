#pragma once

#include <string>

namespace order::ports::output {

/**
 * @brief Результат вызова биллинга
 */
struct BillingResponse {
    bool success{false};
    std::string error;
    int64_t newBalance{0};
};

/**
 * @brief Клиент к Billing Service
 */
class IBillingClient {
public:
    virtual ~IBillingClient() = default;

    /**
     * @brief Списать средства с баланса пользователя
     */
    virtual BillingResponse charge(const std::string& userId, int64_t amount) = 0;
};

} // namespace order::ports::output
