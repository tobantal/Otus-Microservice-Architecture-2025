#include "OrderApp.hpp"

#include <IEnvironment.hpp>
#include <IHttpClient.hpp>
#include <HttpClient.hpp>

// Ports
#include "ports/input/IOrderService.hpp"
#include "ports/output/IOrderRepository.hpp"
#include "ports/output/IBillingClient.hpp"
#include "ports/output/INotificationClient.hpp"
#include "ports/output/IServiceSettings.hpp"
#include "ports/output/IDbSettings.hpp"

// Application
#include "application/OrderService.hpp"

// Secondary Adapters
#include "adapters/secondary/PostgresDbSettings.hpp"
#include "adapters/secondary/PostgresOrderRepository.hpp"
#include "adapters/secondary/HttpBillingClient.hpp"
#include "adapters/secondary/HttpNotificationClient.hpp"
#include "adapters/secondary/BillingClientSettings.hpp"
#include "adapters/secondary/NotificationClientSettings.hpp"

// Primary Adapters
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/primary/OrderHandler.hpp"

#include <boost/di.hpp>
#include <iostream>

namespace di = boost::di;

OrderApp::OrderApp() {
    std::cout << "[OrderApp] Application created" << std::endl;
}

OrderApp::~OrderApp() {
    std::cout << "[OrderApp] Application destroyed" << std::endl;
}

void OrderApp::loadEnvironment(int argc, char* argv[]) {
    std::cout << "[OrderApp] Loading environment..." << std::endl;
    BoostBeastApplication::loadEnvironment(argc, argv);
    std::cout << "[OrderApp] Environment loaded successfully" << std::endl;
}

void OrderApp::configureInjection() {
    std::cout << "[OrderApp] Configuring Boost.DI injection..." << std::endl;

    // ========================================================================
    // Settings - читают из ENV при создании
    // ========================================================================
    auto billingSettings = std::make_shared<order::adapters::secondary::BillingClientSettings>();
    auto notificationSettings = std::make_shared<order::adapters::secondary::NotificationClientSettings>();

    // ========================================================================
    // HTTP Client - общий для всех клиентов
    // ========================================================================
    auto httpClient = std::make_shared<HttpClient>();

    // ========================================================================
    // Boost.DI Injector
    // ========================================================================
    auto injector = di::make_injector(
        di::bind<IEnvironment>().to(env_),

        // Layer 1: Secondary Adapters
        // IDbSettings ← PostgresDbSettings (читает ENV)
        di::bind<order::ports::output::IDbSettings>()
            .to<order::adapters::secondary::PostgresDbSettings>()
            .in(di::singleton),

        // IOrderRepository ← PostgresOrderRepository(IDbSettings)
        di::bind<order::ports::output::IOrderRepository>()
            .to<order::adapters::secondary::PostgresOrderRepository>()
            .in(di::singleton),

        // Billing Client с настройками
        di::bind<order::ports::output::IBillingClient>()
            .to(std::make_shared<order::adapters::secondary::HttpBillingClient>(
                httpClient, billingSettings)),

        // Notification Client с настройками
        di::bind<order::ports::output::INotificationClient>()
            .to(std::make_shared<order::adapters::secondary::HttpNotificationClient>(
                httpClient, notificationSettings)),

        // Layer 2: Application Services
        di::bind<order::ports::input::IOrderService>()
            .to<order::application::OrderService>()
            .in(di::singleton)
    );

    std::cout << "\n📦 Boost.DI Injector configured" << std::endl;
    std::cout << "\n🎮 Registering HTTP Handlers via DI..." << std::endl;

    // Health
    {
        auto handler = injector.create<std::shared_ptr<order::adapters::primary::HealthHandler>>();
        handlers_[getHandlerKey("GET", "/health")] = handler;
        std::cout << "  ✓ HealthHandler: GET /health" << std::endl;
    }

    // Order endpoints
    {
        auto handler = injector.create<std::shared_ptr<order::adapters::primary::OrderHandler>>();
        handlers_[getHandlerKey("POST", "/api/v1/orders")] = handler;
        handlers_[getHandlerKey("GET", "/api/v1/orders/*")] = handler;
        std::cout << "  ✓ OrderHandler: POST /api/v1/orders" << std::endl;
        std::cout << "  ✓ OrderHandler: GET /api/v1/orders/{id}" << std::endl;
    }

    std::cout << "\n[OrderApp] DI configuration completed - "
              << handlers_.size() << " routes registered" << std::endl;
}
