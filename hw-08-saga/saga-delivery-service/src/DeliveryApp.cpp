#include "DeliveryApp.hpp"
#include <boost/di.hpp>

#include "settings/DbSettings.hpp"
#include "settings/RabbitMQSettings.hpp"
#include "ports/output/ISlotRepository.hpp"
#include "ports/output/IEventPublisher.hpp"
#include "ports/output/IEventConsumer.hpp"
#include "adapters/secondary/PostgresSlotRepository.hpp"
#include "adapters/secondary/RabbitMQAdapter.hpp"
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/primary/DeliveryHandler.hpp"
#include "application/DeliveryEventHandler.hpp"

#include <iostream>

namespace di = boost::di;

namespace delivery {

DeliveryApp::DeliveryApp() { std::cout << "[DeliveryApp] Created" << std::endl; }
DeliveryApp::~DeliveryApp() { std::cout << "[DeliveryApp] Destroyed" << std::endl; }

void DeliveryApp::configureInjection() {
    std::cout << "[DeliveryApp] Configuring..." << std::endl;
    
    auto injector = di::make_injector(
        di::bind<settings::DbSettings>.to<settings::DbSettings>(),
        di::bind<settings::RabbitMQSettings>.to<settings::RabbitMQSettings>(),
        di::bind<ports::output::ISlotRepository>.to<adapters::secondary::PostgresSlotRepository>(),
        di::bind<ports::output::IEventPublisher>.to<adapters::secondary::RabbitMQPublisher>(),
        di::bind<ports::output::IEventConsumer>.to<adapters::secondary::RabbitMQConsumer>()
    );
    
    auto healthHandler = std::make_shared<adapters::primary::HealthHandler>();
    auto deliveryHandler = injector.create<std::shared_ptr<adapters::primary::DeliveryHandler>>();
    
    handlers_[getHandlerKey("GET", "/health")] = healthHandler;
    handlers_[getHandlerKey("POST", "/api/v1/delivery/slots")] = deliveryHandler;
    handlers_[getHandlerKey("DELETE", "/api/v1/delivery/slots")] = deliveryHandler;
    
    // Consumer - слушаем warehouse.reserved
    auto consumer = injector.create<std::shared_ptr<ports::output::IEventConsumer>>();
    auto eventHandler = injector.create<std::shared_ptr<application::DeliveryEventHandler>>();
    
    consumer->subscribe(
        {"warehouse.reserved"},
        [eventHandler](const std::string& key, const std::string& msg) {
            eventHandler->handleEvent(key, msg);
        }
    );
    consumer->start();
    
    std::cout << "[DeliveryApp] Configured with RabbitMQ" << std::endl;
}

} // namespace delivery
