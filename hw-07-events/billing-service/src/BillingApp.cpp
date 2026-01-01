#include "BillingApp.hpp"

#include <IEnvironment.hpp>

// Ports
#include "ports/input/IBillingService.hpp"
#include "ports/output/IBillingRepository.hpp"
#include "ports/output/IDbSettings.hpp"

// Application
#include "application/BillingService.hpp"

// Secondary Adapters
#include "adapters/secondary/PostgresDbSettings.hpp"
#include "adapters/secondary/PostgresBillingRepository.hpp"

// Primary Adapters
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/primary/BillingHandler.hpp"

#include <boost/di.hpp>
#include <iostream>

namespace di = boost::di;

BillingApp::BillingApp() {
    std::cout << "[BillingApp] Application created" << std::endl;
}

BillingApp::~BillingApp() {
    std::cout << "[BillingApp] Application destroyed" << std::endl;
}

void BillingApp::loadEnvironment(int argc, char* argv[]) {
    std::cout << "[BillingApp] Loading environment..." << std::endl;
    BoostBeastApplication::loadEnvironment(argc, argv);
    std::cout << "[BillingApp] Environment loaded successfully" << std::endl;
}

void BillingApp::configureInjection() {
    std::cout << "[BillingApp] Configuring Boost.DI injection..." << std::endl;

    // ========================================================================
    // Boost.DI Injector Configuration
    // ========================================================================

    auto injector = di::make_injector(
        // IEnvironment
        di::bind<IEnvironment>().to(env_),

        // Layer 1: Secondary Adapters
        // IDbSettings ← PostgresDbSettings (читает ENV)
        di::bind<billing::ports::output::IDbSettings>()
            .to<billing::adapters::secondary::PostgresDbSettings>()
            .in(di::singleton),

        // IBillingRepository ← PostgresBillingRepository(IDbSettings)
        di::bind<billing::ports::output::IBillingRepository>()
            .to<billing::adapters::secondary::PostgresBillingRepository>()
            .in(di::singleton),

        // Layer 2: Application Services
        di::bind<billing::ports::input::IBillingService>()
            .to<billing::application::BillingService>()
            .in(di::singleton)
    );

    std::cout << "\n📦 Boost.DI Injector configured" << std::endl;

    // ========================================================================
    // Layer 3: Primary Adapters (HTTP Handlers)
    // ========================================================================

    std::cout << "\n🎮 Registering HTTP Handlers via DI..." << std::endl;

    // Health
    {
        auto handler = injector.create<std::shared_ptr<billing::adapters::primary::HealthHandler>>();
        handlers_[getHandlerKey("GET", "/health")] = handler;
        std::cout << "  ✓ HealthHandler: GET /health" << std::endl;
    }

    // Billing endpoints
    {
        auto handler = injector.create<std::shared_ptr<billing::adapters::primary::BillingHandler>>();
        handlers_[getHandlerKey("POST", "/api/v1/billing/accounts")] = handler;
        handlers_[getHandlerKey("POST", "/api/v1/billing/deposit")] = handler;
        handlers_[getHandlerKey("POST", "/api/v1/billing/charge")] = handler;
        handlers_[getHandlerKey("GET", "/api/v1/billing/accounts/*")] = handler;
        std::cout << "  ✓ BillingHandler: POST /api/v1/billing/accounts" << std::endl;
        std::cout << "  ✓ BillingHandler: POST /api/v1/billing/deposit" << std::endl;
        std::cout << "  ✓ BillingHandler: POST /api/v1/billing/charge" << std::endl;
        std::cout << "  ✓ BillingHandler: GET /api/v1/billing/accounts/{id}" << std::endl;
    }

    std::cout << "\n[BillingApp] DI configuration completed - "
              << handlers_.size() << " routes registered" << std::endl;
}
