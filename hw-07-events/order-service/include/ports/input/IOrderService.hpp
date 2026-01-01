#pragma once

#include "domain/Order.hpp"
#include <optional>
#include <string>

namespace order::ports::input {

/**
 * @brief Результат создания заказа
 */
struct CreateOrderResult {
    bool success{false};
    std::string error;
    domain::Order order;
};

/**
 * @brief Интерфейс сервиса заказов
 */
class IOrderService {
public:
    virtual ~IOrderService() = default;

    /**
     * @brief Создать заказ
     * 
     * 1. Списывает деньги через Billing
     * 2. Отправляет уведомление через Notification
     */
    virtual CreateOrderResult createOrder(const std::string& userId, int64_t amount) = 0;

    /**
     * @brief Получить заказ по ID
     */
    virtual std::optional<domain::Order> getOrder(const std::string& orderId) = 0;
};

} // namespace order::ports::input
