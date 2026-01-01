#pragma once

#include <BoostBeastApplication.hpp>

/**
 * @brief Order Service приложение для HW07
 * 
 * Координирует создание заказов с Billing и Notification.
 * 
 * Endpoints:
 * - GET  /health            - Health check
 * - POST /api/v1/orders     - Создать заказ
 * - GET  /api/v1/orders/{id} - Получить заказ
 */
class OrderApp : public BoostBeastApplication {
public:
    OrderApp();
    ~OrderApp() override;

protected:
    void loadEnvironment(int argc, char* argv[]) override;
    void configureInjection() override;
};
