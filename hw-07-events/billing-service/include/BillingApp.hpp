#pragma once

#include <BoostBeastApplication.hpp>

/**
 * @brief Billing Service приложение для HW07
 * 
 * Управляет балансами пользователей.
 * 
 * Endpoints:
 * - GET  /health                       - Health check
 * - POST /api/v1/billing/accounts      - Создать аккаунт
 * - POST /api/v1/billing/deposit       - Пополнить
 * - POST /api/v1/billing/charge        - Списать
 * - GET  /api/v1/billing/accounts/{id} - Баланс
 */
class BillingApp : public BoostBeastApplication {
public:
    BillingApp();
    ~BillingApp() override;

protected:
    void loadEnvironment(int argc, char* argv[]) override;
    void configureInjection() override;
};
