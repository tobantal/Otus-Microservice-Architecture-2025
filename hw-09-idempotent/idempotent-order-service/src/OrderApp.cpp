#include "OrderApp.hpp"
#include <boost/di.hpp>

#include "settings/DbSettings.hpp"
#include "settings/RabbitMQSettings.hpp"
#include "ports/input/IOrderService.hpp"
#include "ports/output/IOrderRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include "ports/output/IEventConsumer.hpp"
#include "ports/output/IIdempotencyRepository.hpp"
#include "adapters/secondary/PostgresOrderRepository.hpp"
#include "adapters/secondary/PostgresIdempotencyRepository.hpp"
#include "adapters/secondary/RabbitMQAdapter.hpp"
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/primary/OrderHandler.hpp"
#include "adapters/primary/IdempotentHandler.hpp"
#include "application/OrderService.hpp"

#include <nlohmann/json.hpp>
#include <iostream>

namespace di = boost::di;

namespace order {

OrderApp::OrderApp() { std::cout << "[OrderApp] Created" << std::endl; }
OrderApp::~OrderApp() { std::cout << "[OrderApp] Destroyed" << std::endl; }

void OrderApp::configureInjection() {
    std::cout << "[OrderApp] Configuring..." << std::endl;
    
    auto injector = di::make_injector(
        di::bind<settings::DbSettings>.to<settings::DbSettings>(),
        di::bind<settings::RabbitMQSettings>.to<settings::RabbitMQSettings>(),
        di::bind<ports::output::IOrderRepository>.to<adapters::secondary::PostgresOrderRepository>(),
        di::bind<ports::output::IIdempotencyRepository>.to<adapters::secondary::PostgresIdempotencyRepository>(),
        di::bind<ports::output::IEventPublisher>.to<adapters::secondary::RabbitMQPublisher>(),
        di::bind<ports::output::IEventConsumer>.to<adapters::secondary::RabbitMQConsumer>(),
        di::bind<ports::input::IOrderService>.to<application::OrderService>()
    );
    
    // Handlers
    auto healthHandler = std::make_shared<adapters::primary::HealthHandler>();
    auto orderHandler = injector.create<std::shared_ptr<adapters::primary::OrderHandler>>();
    auto idempotencyRepo = injector.create<std::shared_ptr<ports::output::IIdempotencyRepository>>();
    
    // Декоратор для идемпотентности
    auto idempotentOrderHandler = std::make_shared<adapters::primary::IdempotentHandler>(
        orderHandler, idempotencyRepo);
    
    handlers_[getHandlerKey("GET", "/health")] = healthHandler;
    handlers_[getHandlerKey("POST", "/api/v1/orders")] = idempotentOrderHandler;
    handlers_[getHandlerKey("GET", "/api/v1/orders/*")] = orderHandler;
    
    // Consumer - слушаем события от других сервисов
    auto consumer = injector.create<std::shared_ptr<ports::output::IEventConsumer>>();
    auto orderService = injector.create<std::shared_ptr<application::OrderService>>();
    
    consumer->subscribe(
        {"billing.charged", "billing.failed", "billing.refunded",
         "warehouse.reserved", "warehouse.failed", "warehouse.released",
         "delivery.booked", "delivery.failed"},
        [orderService](const std::string& key, const std::string& msg) {
            auto json = nlohmann::json::parse(msg);
            std::string orderId = json["order_id"];
            std::string reason = json.value("reason", "");
            
            if (key == "billing.charged") orderService->onBillingCharged(orderId);
            else if (key == "billing.failed") orderService->onBillingFailed(orderId, reason);
            else if (key == "warehouse.reserved") orderService->onWarehouseReserved(orderId);
            else if (key == "warehouse.failed") orderService->onWarehouseFailed(orderId, reason);
            else if (key == "delivery.booked") orderService->onDeliveryBooked(orderId);
            else if (key == "delivery.failed") orderService->onDeliveryFailed(orderId, reason);
        }
    );
    consumer->start();
    
    std::cout << "[OrderApp] Configured with RabbitMQ consumer and Idempotency" << std::endl;
}

} // namespace order
