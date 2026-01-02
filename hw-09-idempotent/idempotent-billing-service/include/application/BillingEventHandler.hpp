#pragma once

#include "ports/output/IAccountRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace billing::application {

class BillingEventHandler {
public:
    BillingEventHandler(std::shared_ptr<ports::output::IAccountRepository> repo,
                        std::shared_ptr<ports::output::IEventPublisher> publisher)
        : repo_(repo), publisher_(publisher) {
        std::cout << "[BillingEventHandler] Created" << std::endl;
    }
    
    void handleEvent(const std::string& routingKey, const std::string& message) {
        auto json = nlohmann::json::parse(message);
        std::string orderId = json["order_id"];
        std::string userId = json["user_id"];
        int64_t amount = json.value("amount", int64_t{0});
        std::string productId = json.value("product_id", "");
        int32_t quantity = json.value("quantity", 1);
        
        if (routingKey == "order.created") {
            charge(orderId, userId, amount, productId, quantity);
        }
        else if (routingKey == "warehouse.failed" || routingKey == "delivery.failed") {
            refund(orderId, userId, amount);
        }
    }

private:
    std::shared_ptr<ports::output::IAccountRepository> repo_;
    std::shared_ptr<ports::output::IEventPublisher> publisher_;
    
    void charge(const std::string& orderId, const std::string& userId, int64_t amount,
                const std::string& productId, int32_t quantity) {
        auto account = repo_->findByUserId(userId);
        
        if (!account) {
            publishFailed(orderId, userId, amount, productId, quantity, "Account not found");
            return;
        }
        
        if (account->balance < amount) {
            publishFailed(orderId, userId, amount, productId, quantity, "Insufficient funds");
            return;
        }
        
        repo_->updateBalance(userId, account->balance - amount);
        
        nlohmann::json event = {
            {"order_id", orderId},
            {"user_id", userId},
            {"amount", amount},
            {"product_id", productId},
            {"quantity", quantity}
        };
        publisher_->publish("billing.charged", event.dump());
        std::cout << "[Billing] Charged " << amount << " from " << userId << std::endl;
    }
    
    void refund(const std::string& orderId, const std::string& userId, int64_t amount) {
        auto account = repo_->findByUserId(userId);
        if (account) {
            repo_->updateBalance(userId, account->balance + amount);
        }
        
        nlohmann::json event = {
            {"order_id", orderId},
            {"user_id", userId},
            {"amount", amount}
        };
        publisher_->publish("billing.refunded", event.dump());
        std::cout << "[Billing] Refunded " << amount << " to " << userId << std::endl;
    }
    
    void publishFailed(const std::string& orderId, const std::string& userId, 
                       int64_t amount, const std::string& productId, int32_t quantity,
                       const std::string& reason) {
        nlohmann::json event = {
            {"order_id", orderId},
            {"user_id", userId},
            {"amount", amount},
            {"product_id", productId},
            {"quantity", quantity},
            {"reason", reason}
        };
        publisher_->publish("billing.failed", event.dump());
        std::cout << "[Billing] Failed: " << reason << std::endl;
    }
};

} // namespace billing::application
