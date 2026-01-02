#pragma once

#include "ports/output/IProductRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace warehouse::application {

class WarehouseEventHandler {
public:
    WarehouseEventHandler(std::shared_ptr<ports::output::IProductRepository> repo,
                          std::shared_ptr<ports::output::IEventPublisher> publisher)
        : repo_(repo), publisher_(publisher) {
        std::cout << "[WarehouseEventHandler] Created" << std::endl;
    }
    
    void handleEvent(const std::string& routingKey, const std::string& message) {
        std::cout << "[WarehouseEventHandler] Received: " << routingKey << " -> " << message << std::endl;
        
        auto json = nlohmann::json::parse(message);
        std::string orderId = json["order_id"];
        std::string userId = json["user_id"];
        std::string productId = json.value("product_id", "");
        int32_t quantity = json.value("quantity", 1);
        int64_t amount = json.value("amount", int64_t{0});
        
        std::cout << "[WarehouseEventHandler] productId=" << productId << ", quantity=" << quantity << std::endl;
        
        if (routingKey == "billing.charged") {
            reserve(orderId, userId, productId, quantity, amount);
        }
        else if (routingKey == "delivery.failed") {
            release(orderId, userId, productId, quantity, amount);
        }
    }

private:
    std::shared_ptr<ports::output::IProductRepository> repo_;
    std::shared_ptr<ports::output::IEventPublisher> publisher_;
    
    void reserve(const std::string& orderId, const std::string& userId,
                const std::string& productId, int32_t quantity, int64_t amount) {
        auto product = repo_->findById(productId);
        
        if (!product) {
            publishFailed(orderId, userId, productId, quantity, amount, "Product not found");
            return;
        }
        
        int32_t available = product->stock - product->reserved;
        if (available < quantity) {
            publishFailed(orderId, userId, productId, quantity, amount, "Insufficient stock");
            return;
        }
        
        repo_->updateStock(productId, product->stock, product->reserved + quantity);
        
        nlohmann::json event = {
            {"order_id", orderId},
            {"user_id", userId},
            {"product_id", productId},
            {"quantity", quantity},
            {"amount", amount}
        };
        publisher_->publish("warehouse.reserved", event.dump());
        std::cout << "[Warehouse] Reserved " << quantity << " of " << productId << std::endl;
    }
    
    void release(const std::string& orderId, const std::string& userId,
                const std::string& productId, int32_t quantity, int64_t amount) {
        auto product = repo_->findById(productId);
        if (product) {
            int32_t newReserved = std::max(0, product->reserved - quantity);
            repo_->updateStock(productId, product->stock, newReserved);
        }
        
        nlohmann::json event = {
            {"order_id", orderId},
            {"user_id", userId},
            {"product_id", productId},
            {"quantity", quantity},
            {"amount", amount}
        };
        publisher_->publish("warehouse.released", event.dump());
        std::cout << "[Warehouse] Released " << quantity << " of " << productId << std::endl;
    }
    
    void publishFailed(const std::string& orderId, const std::string& userId,
                       const std::string& productId, int32_t quantity, int64_t amount,
                       const std::string& reason) {
        nlohmann::json event = {
            {"order_id", orderId},
            {"user_id", userId},
            {"product_id", productId},
            {"quantity", quantity},
            {"amount", amount},
            {"reason", reason}
        };
        publisher_->publish("warehouse.failed", event.dump());
        std::cout << "[Warehouse] Failed: " << reason << std::endl;
    }
};

} // namespace warehouse::application
