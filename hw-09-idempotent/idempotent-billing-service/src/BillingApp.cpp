#include "BillingApp.hpp"
#include <boost/di.hpp>

#include "settings/DbSettings.hpp"
#include "settings/RabbitMQSettings.hpp"
#include "ports/output/IAccountRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include "ports/output/IEventConsumer.hpp"
#include "adapters/secondary/PostgresAccountRepository.hpp"
#include "adapters/secondary/RabbitMQAdapter.hpp"
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/primary/BillingHandler.hpp"
#include "application/BillingEventHandler.hpp"

#include <iostream>

namespace di = boost::di;

namespace billing {

BillingApp::BillingApp() { std::cout << "[BillingApp] Created" << std::endl; }
BillingApp::~BillingApp() { std::cout << "[BillingApp] Destroyed" << std::endl; }

void BillingApp::configureInjection() {
    std::cout << "[BillingApp] Configuring..." << std::endl;
    
    auto injector = di::make_injector(
        di::bind<settings::DbSettings>.to<settings::DbSettings>(),
        di::bind<settings::RabbitMQSettings>.to<settings::RabbitMQSettings>(),
        di::bind<ports::output::IAccountRepository>.to<adapters::secondary::PostgresAccountRepository>(),
        di::bind<ports::output::IEventPublisher>.to<adapters::secondary::RabbitMQPublisher>(),
        di::bind<ports::output::IEventConsumer>.to<adapters::secondary::RabbitMQConsumer>()
    );
    
    auto healthHandler = std::make_shared<adapters::primary::HealthHandler>();
    auto billingHandler = injector.create<std::shared_ptr<adapters::primary::BillingHandler>>();
    
    handlers_[getHandlerKey("GET", "/health")] = healthHandler;
    handlers_[getHandlerKey("POST", "/api/v1/billing/accounts")] = billingHandler;
    handlers_[getHandlerKey("POST", "/api/v1/billing/deposit")] = billingHandler;
    handlers_[getHandlerKey("GET", "/api/v1/billing/accounts/*")] = billingHandler;
    
    // Event consumer - слушаем order.created, warehouse.failed, delivery.failed
    auto consumer = injector.create<std::shared_ptr<ports::output::IEventConsumer>>();
    auto eventHandler = injector.create<std::shared_ptr<application::BillingEventHandler>>();
    
    consumer->subscribe(
        {"order.created", "warehouse.failed", "delivery.failed"},
        [eventHandler](const std::string& key, const std::string& msg) {
            eventHandler->handleEvent(key, msg);
        }
    );
    consumer->start();
    
    std::cout << "[BillingApp] Configured with RabbitMQ" << std::endl;
}

} // namespace billing
