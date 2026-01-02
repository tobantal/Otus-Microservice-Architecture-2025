#pragma once

#include "ports/input/IOrderService.hpp"
#include "ports/output/IOrderRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <chrono>
#include <iostream>

namespace order::application {

class OrderService : public ports::input::IOrderService {
public:
    OrderService(std::shared_ptr<ports::output::IOrderRepository> repo,
                 std::shared_ptr<ports::output::IEventPublisher> publisher)
        : repo_(repo), publisher_(publisher) {
        std::cout << "[OrderService] Created" << std::endl;
    }
    
    std::string createOrder(const std::string& userId, const std::string& productId,
                           int64_t amount, int32_t quantity) override {
        domain::Order order;
        order.id = generateId();
        order.userId = userId;
        order.productId = productId;
        order.amount = amount;
        order.quantity = quantity;
        order.state = domain::SagaState::CREATED;
        
        repo_->save(order);
        
        // Публикуем событие - Billing подпишется и обработает
        nlohmann::json event = {
            {"order_id", order.id},
            {"user_id", userId},
            {"product_id", productId},
            {"amount", amount},
            {"quantity", quantity}
        };
        publisher_->publish("order.created", event.dump());
        
        repo_->updateState(order.id, domain::SagaState::BILLING_PENDING);
        
        std::cout << "[OrderService] Order created: " << order.id << std::endl;
        return order.id;
    }
    
    std::optional<domain::Order> getOrder(const std::string& orderId) override {
        return repo_->findById(orderId);
    }
    
    // Вызывается из EventHandler при получении событий
    void onBillingCharged(const std::string& orderId) {
        repo_->updateState(orderId, domain::SagaState::WAREHOUSE_PENDING);
        std::cout << "[OrderService] Billing charged for: " << orderId << std::endl;
    }
    
    void onBillingFailed(const std::string& orderId, const std::string& reason) {
        repo_->updateState(orderId, domain::SagaState::FAILED, reason);
        std::cout << "[OrderService] Billing failed for: " << orderId << std::endl;
    }
    
    void onWarehouseReserved(const std::string& orderId) {
        repo_->updateState(orderId, domain::SagaState::DELIVERY_PENDING);
        std::cout << "[OrderService] Warehouse reserved for: " << orderId << std::endl;
    }
    
    void onWarehouseFailed(const std::string& orderId, const std::string& reason) {
        repo_->updateState(orderId, domain::SagaState::FAILED, reason);
        std::cout << "[OrderService] Warehouse failed for: " << orderId << std::endl;
    }
    
    void onDeliveryBooked(const std::string& orderId) {
        repo_->updateState(orderId, domain::SagaState::COMPLETED);
        std::cout << "[OrderService] SAGA COMPLETED for: " << orderId << std::endl;
    }
    
    void onDeliveryFailed(const std::string& orderId, const std::string& reason) {
        repo_->updateState(orderId, domain::SagaState::FAILED, reason);
        std::cout << "[OrderService] Delivery failed for: " << orderId << std::endl;
    }

private:
    std::shared_ptr<ports::output::IOrderRepository> repo_;
    std::shared_ptr<ports::output::IEventPublisher> publisher_;
    
    std::string generateId() {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        static int c = 0;
        return "order-" + std::to_string(ms) + "-" + std::to_string(++c);
    }
};

} // namespace order::application
