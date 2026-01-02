#include "WarehouseApp.hpp"
#include <boost/di.hpp>

#include "settings/DbSettings.hpp"
#include "settings/RabbitMQSettings.hpp"
#include "ports/output/IProductRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include "ports/output/IEventConsumer.hpp"
#include "adapters/secondary/PostgresProductRepository.hpp"
#include "adapters/secondary/RabbitMQAdapter.hpp"
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/primary/WarehouseHandler.hpp"
#include "application/WarehouseEventHandler.hpp"

#include <iostream>

namespace di = boost::di;

namespace warehouse {

WarehouseApp::WarehouseApp() { std::cout << "[WarehouseApp] Created" << std::endl; }
WarehouseApp::~WarehouseApp() { std::cout << "[WarehouseApp] Destroyed" << std::endl; }

void WarehouseApp::configureInjection() {
    std::cout << "[WarehouseApp] Configuring..." << std::endl;
    
    auto injector = di::make_injector(
        di::bind<settings::DbSettings>.to<settings::DbSettings>(),
        di::bind<settings::RabbitMQSettings>.to<settings::RabbitMQSettings>(),
        di::bind<ports::output::IProductRepository>.to<adapters::secondary::PostgresProductRepository>(),
        di::bind<ports::output::IEventPublisher>.to<adapters::secondary::RabbitMQPublisher>(),
        di::bind<ports::output::IEventConsumer>.to<adapters::secondary::RabbitMQConsumer>()
    );
    
    auto healthHandler = std::make_shared<adapters::primary::HealthHandler>();
    auto warehouseHandler = injector.create<std::shared_ptr<adapters::primary::WarehouseHandler>>();
    
    handlers_[getHandlerKey("GET", "/health")] = healthHandler;
    handlers_[getHandlerKey("POST", "/api/v1/warehouse/products")] = warehouseHandler;
    handlers_[getHandlerKey("GET", "/api/v1/warehouse/stock/*")] = warehouseHandler;
    
    // Consumer - слушаем billing.charged, delivery.failed
    auto consumer = injector.create<std::shared_ptr<ports::output::IEventConsumer>>();
    auto eventHandler = injector.create<std::shared_ptr<application::WarehouseEventHandler>>();
    
    consumer->subscribe(
        {"billing.charged", "delivery.failed"},
        [eventHandler](const std::string& key, const std::string& msg) {
            eventHandler->handleEvent(key, msg);
        }
    );
    consumer->start();
    
    std::cout << "[WarehouseApp] Configured with RabbitMQ" << std::endl;
}

} // namespace warehouse
