#include "NotificationApp.hpp"

#include <IEnvironment.hpp>

// Ports
#include "ports/input/INotificationService.hpp"
#include "ports/output/INotificationRepository.hpp"
#include "ports/output/IDbSettings.hpp"

// Application
#include "application/NotificationService.hpp"

// Secondary Adapters
#include "adapters/secondary/PostgresDbSettings.hpp"
#include "adapters/secondary/PostgresNotificationRepository.hpp"

// Primary Adapters
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/primary/NotificationHandler.hpp"

#include <boost/di.hpp>
#include <iostream>

namespace di = boost::di;

NotificationApp::NotificationApp() {
    std::cout << "[NotificationApp] Application created" << std::endl;
}

NotificationApp::~NotificationApp() {
    std::cout << "[NotificationApp] Application destroyed" << std::endl;
}

void NotificationApp::loadEnvironment(int argc, char* argv[]) {
    std::cout << "[NotificationApp] Loading environment..." << std::endl;
    BoostBeastApplication::loadEnvironment(argc, argv);
    std::cout << "[NotificationApp] Environment loaded successfully" << std::endl;
}

void NotificationApp::configureInjection() {
    std::cout << "[NotificationApp] Configuring Boost.DI injection..." << std::endl;

    auto injector = di::make_injector(
        di::bind<IEnvironment>().to(env_),

        // Layer 1: Secondary Adapters
        // IDbSettings ← PostgresDbSettings (читает ENV)
        di::bind<notification::ports::output::IDbSettings>()
            .to<notification::adapters::secondary::PostgresDbSettings>()
            .in(di::singleton),

        // INotificationRepository ← PostgresNotificationRepository(IDbSettings)
        di::bind<notification::ports::output::INotificationRepository>()
            .to<notification::adapters::secondary::PostgresNotificationRepository>()
            .in(di::singleton),

        // Layer 2: Application Services
        di::bind<notification::ports::input::INotificationService>()
            .to<notification::application::NotificationService>()
            .in(di::singleton)
    );

    std::cout << "\n📦 Boost.DI Injector configured" << std::endl;
    std::cout << "\n🎮 Registering HTTP Handlers via DI..." << std::endl;

    // Health
    {
        auto handler = injector.create<std::shared_ptr<notification::adapters::primary::HealthHandler>>();
        handlers_[getHandlerKey("GET", "/health")] = handler;
        std::cout << "  ✓ HealthHandler: GET /health" << std::endl;
    }

    // Notification endpoints
    {
        auto handler = injector.create<std::shared_ptr<notification::adapters::primary::NotificationHandler>>();
        handlers_[getHandlerKey("POST", "/api/v1/notifications")] = handler;
        handlers_[getHandlerKey("GET", "/api/v1/notifications")] = handler;
        std::cout << "  ✓ NotificationHandler: POST /api/v1/notifications" << std::endl;
        std::cout << "  ✓ NotificationHandler: GET /api/v1/notifications" << std::endl;
    }

    std::cout << "\n[NotificationApp] DI configuration completed - "
              << handlers_.size() << " routes registered" << std::endl;
}
