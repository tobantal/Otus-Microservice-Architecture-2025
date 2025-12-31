#include "AuthApp.hpp"

#include <IEnvironment.hpp>

// Domain
#include "domain/User.hpp"
#include "domain/Profile.hpp"

// Ports
#include "ports/output/IDbSettings.hpp"
#include "ports/output/IJwtProvider.hpp"
#include "ports/output/IUserRepository.hpp"
#include "ports/output/IProfileRepository.hpp"
#include "ports/input/IAuthService.hpp"

// Application Services
#include "application/AuthService.hpp"

// Secondary Adapters
#include "adapters/secondary/PostgresDbSettings.hpp"
#include "adapters/secondary/FakeJwtAdapter.hpp"
#include "adapters/secondary/PostgresUserRepository.hpp"
#include "adapters/secondary/PostgresProfileRepository.hpp"

// Primary Adapters
#include "adapters/primary/HealthCheckHandler.hpp"
#include "adapters/primary/RegisterHandler.hpp"
#include "adapters/primary/LoginHandler.hpp"
#include "adapters/primary/LogoutHandler.hpp"
#include "adapters/primary/ProfileHandler.hpp"

#include <boost/di.hpp>
#include <iostream>

namespace di = boost::di;

// ============================================================================
// Конфигурационные константы для DI
// ============================================================================
namespace config {
    constexpr int JWT_LIFETIME_SECONDS = 86400; // 24 часа
}

// ============================================================================
// AuthApp Implementation
// ============================================================================

AuthApp::AuthApp() {
    std::cout << "[AuthApp] Application created" << std::endl;
}

AuthApp::~AuthApp() {
    std::cout << "[AuthApp] Application destroyed" << std::endl;
}

void AuthApp::loadEnvironment(int argc, char* argv[]) {
    std::cout << "[AuthApp] Loading environment..." << std::endl;
    
    // Вызываем базовый метод который загружает config.json в env_
    BoostBeastApplication::loadEnvironment(argc, argv);
    
    std::cout << "[AuthApp] Environment loaded successfully" << std::endl;
}

void AuthApp::configureInjection() {
    std::cout << "[AuthApp] Configuring Boost.DI injection..." << std::endl;

    // ========================================================================
    // Boost.DI Injector Configuration
    // ========================================================================

    auto injector = di::make_injector(

        // ====================================================================
        // Layer 1: Secondary Adapters (Output Ports implementations)
        // ====================================================================

        // IEnvironment
        di::bind<IEnvironment>().to(env_),

        // IDbSettings ← PostgresDbSettings (читает env variables)
        di::bind<ports::output::IDbSettings>()
            .to<adapters::secondary::PostgresDbSettings>()
            .in(di::singleton),

        // IJwtProvider ← FakeJwtAdapter
        di::bind<ports::output::IJwtProvider>()
            .to(std::make_shared<adapters::secondary::FakeJwtAdapter>(
                config::JWT_LIFETIME_SECONDS)),

        // IUserRepository ← PostgresUserRepository(IDbSettings)
        di::bind<ports::output::IUserRepository>()
            .to<adapters::secondary::PostgresUserRepository>()
            .in(di::singleton),

        // IProfileRepository ← PostgresProfileRepository(IDbSettings)
        di::bind<ports::output::IProfileRepository>()
            .to<adapters::secondary::PostgresProfileRepository>()
            .in(di::singleton),

        // ====================================================================
        // Layer 2: Application Services (Input Ports implementations)
        // ====================================================================

        di::bind<ports::input::IAuthService>()
            .to<application::AuthService>()
            .in(di::singleton)
    );

    std::cout << "\n📦 Boost.DI Injector configured:" << std::endl;
    std::cout << "  ✓ Secondary Adapters (4 bindings)" << std::endl;
    std::cout << "  ✓ Application Services (1 binding)" << std::endl;

    // ========================================================================
    // Layer 3: Primary Adapters (HTTP Handlers)
    // ========================================================================

    std::cout << "\n🎮 Registering HTTP Handlers via DI..." << std::endl;

    // Health check
    {
        auto handler = injector.create<std::shared_ptr<adapters::primary::HealthCheckHandler>>();
        handlers_[getHandlerKey("GET", "/health")] = handler;
        std::cout << "  ✓ HealthCheckHandler: GET /health" << std::endl;
    }

    // Register
    {
        auto handler = injector.create<std::shared_ptr<adapters::primary::RegisterHandler>>();
        handlers_[getHandlerKey("POST", "/api/v1/auth/register")] = handler;
        std::cout << "  ✓ RegisterHandler: POST /api/v1/auth/register" << std::endl;
    }

    // Login
    {
        auto handler = injector.create<std::shared_ptr<adapters::primary::LoginHandler>>();
        handlers_[getHandlerKey("POST", "/api/v1/auth/login")] = handler;
        std::cout << "  ✓ LoginHandler: POST /api/v1/auth/login" << std::endl;
    }

    // Logout
    {
        auto handler = injector.create<std::shared_ptr<adapters::primary::LogoutHandler>>();
        handlers_[getHandlerKey("POST", "/api/v1/auth/logout")] = handler;
        std::cout << "  ✓ LogoutHandler: POST /api/v1/auth/logout" << std::endl;
    }

    // Profile (GET и PUT)
    {
        auto handler = injector.create<std::shared_ptr<adapters::primary::ProfileHandler>>();
        handlers_[getHandlerKey("GET", "/api/v1/profile")] = handler;
        handlers_[getHandlerKey("PUT", "/api/v1/profile")] = handler;
        std::cout << "  ✓ ProfileHandler: GET/PUT /api/v1/profile" << std::endl;
    }

    std::cout << "\n[AuthApp] DI configuration completed - "
              << handlers_.size() << " routes registered" << std::endl;
}
