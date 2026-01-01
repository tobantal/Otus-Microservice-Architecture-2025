#pragma once

#include "domain/BillingAccount.hpp"
#include <optional>
#include <string>

namespace billing::ports::input {

/**
 * @brief Результат операции биллинга
 */
struct BillingResult {
    bool success{false};
    std::string error;
    int64_t newBalance{0};
};

/**
 * @brief Интерфейс сервиса биллинга
 */
class IBillingService {
public:
    virtual ~IBillingService() = default;

    /**
     * @brief Создать аккаунт для пользователя
     */
    virtual BillingResult createAccount(const std::string& userId) = 0;

    /**
     * @brief Пополнить баланс
     */
    virtual BillingResult deposit(const std::string& userId, int64_t amount) = 0;

    /**
     * @brief Списать с баланса
     * @return success=false если недостаточно средств
     */
    virtual BillingResult charge(const std::string& userId, int64_t amount) = 0;

    /**
     * @brief Получить баланс
     */
    virtual std::optional<domain::BillingAccount> getAccount(const std::string& userId) = 0;
};

} // namespace billing::ports::input
