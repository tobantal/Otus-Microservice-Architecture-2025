#pragma once

#include "ports/output/ISlotRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace delivery::application {

class DeliveryEventHandler {
public:
    DeliveryEventHandler(std::shared_ptr<ports::output::ISlotRepository> repo,
                         std::shared_ptr<ports::output::IEventPublisher> publisher)
        : repo_(repo), publisher_(publisher) {
        std::cout << "[DeliveryEventHandler] Created" << std::endl;
    }
    
    void handleEvent(const std::string& routingKey, const std::string& message) {
        auto json = nlohmann::json::parse(message);
        std::string orderId = json["order_id"];
        std::string userId = json["user_id"];
        std::string productId = json.value("product_id", "");
        int32_t quantity = json.value("quantity", 1);
        int64_t amount = json.value("amount", int64_t{0});
        
        if (routingKey == "warehouse.reserved") {
            book(orderId, userId, productId, quantity, amount);
        }
    }

private:
    std::shared_ptr<ports::output::ISlotRepository> repo_;
    std::shared_ptr<ports::output::IEventPublisher> publisher_;
    
    void book(const std::string& orderId, const std::string& userId,
              const std::string& productId, int32_t quantity, int64_t amount) {
        auto slot = repo_->findAvailable();
        
        if (!slot) {
            nlohmann::json event = {
                {"order_id", orderId},
                {"user_id", userId},
                {"product_id", productId},
                {"quantity", quantity},
                {"amount", amount},
                {"reason", "No available delivery slots"}
            };
            publisher_->publish("delivery.failed", event.dump());
            std::cout << "[Delivery] No slots available" << std::endl;
            return;
        }
        
        repo_->book(slot->id, orderId);
        
        nlohmann::json event = {
            {"order_id", orderId},
            {"user_id", userId},
            {"slot_id", slot->id}
        };
        publisher_->publish("delivery.booked", event.dump());
        std::cout << "[Delivery] Booked slot " << slot->id << " for order " << orderId << std::endl;
    }
};

} // namespace delivery::application
