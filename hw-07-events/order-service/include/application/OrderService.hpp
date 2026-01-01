#pragma once

#include "ports/input/IOrderService.hpp"
#include "ports/output/IOrderRepository.hpp"
#include "ports/output/IBillingClient.hpp"
#include "ports/output/INotificationClient.hpp"
#include <memory>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace order::application {

/**
 * @brief Реализация сервиса заказов
 * 
 * Координирует взаимодействие между Billing и Notification.
 */
class OrderService : public ports::input::IOrderService {
public:
    OrderService(
        std::shared_ptr<ports::output::IOrderRepository> repository,
        std::shared_ptr<ports::output::IBillingClient> billingClient,
        std::shared_ptr<ports::output::INotificationClient> notificationClient
    ) : repository_(std::move(repository))
      , billingClient_(std::move(billingClient))
      , notificationClient_(std::move(notificationClient))
    {
        std::cout << "[OrderService] Created" << std::endl;
    }

    ports::input::CreateOrderResult createOrder(const std::string& userId, int64_t amount) override {
        std::cout << "[OrderService] Creating order for user: " << userId 
                  << ", amount: " << amount << std::endl;

        // Создаём заказ
        domain::Order order;
        order.id = generateId();
        order.userId = userId;
        order.amount = amount;
        order.status = domain::OrderStatus::CREATED;
        order.createdAt = getCurrentTimestamp();

        // 1. Списываем деньги через Billing
        auto billingResult = billingClient_->charge(userId, amount);

        if (!billingResult.success) {
            // Недостаточно средств или ошибка
            order.status = domain::OrderStatus::FAILED;
            repository_->save(order);

            // Отправляем "письмо горя"
            std::string message = "Order " + order.id + " failed: " + billingResult.error;
            notificationClient_->send(userId, "order.failed", message);

            std::cout << "[OrderService] Order failed: " << billingResult.error << std::endl;
            return {false, billingResult.error, order};
        }

        // Успех - обновляем статус
        order.status = domain::OrderStatus::PAID;
        repository_->save(order);

        // 2. Отправляем "письмо счастья"
        std::string message = "Order " + order.id + " completed! Amount: " + std::to_string(amount) + 
                             ". New balance: " + std::to_string(billingResult.newBalance);
        notificationClient_->send(userId, "order.success", message);

        std::cout << "[OrderService] Order completed: " << order.id << std::endl;
        return {true, "", order};
    }

    std::optional<domain::Order> getOrder(const std::string& orderId) override {
        return repository_->findById(orderId);
    }

private:
    std::shared_ptr<ports::output::IOrderRepository> repository_;
    std::shared_ptr<ports::output::IBillingClient> billingClient_;
    std::shared_ptr<ports::output::INotificationClient> notificationClient_;

    std::string generateId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(8) << (dis(gen) & 0xFFFFFFFF);
        ss << "-";
        ss << std::setw(4) << (dis(gen) & 0xFFFF);
        ss << "-4";
        ss << std::setw(3) << (dis(gen) & 0xFFF);
        ss << "-";
        ss << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000);
        ss << "-";
        ss << std::setw(12) << (dis(gen) & 0xFFFFFFFFFFFF);
        return ss.str();
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

} // namespace order::application
