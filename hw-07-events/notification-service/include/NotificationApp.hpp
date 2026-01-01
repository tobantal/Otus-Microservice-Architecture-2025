#pragma once

#include <BoostBeastApplication.hpp>

/**
 * @brief Notification Service приложение для HW07
 * 
 * Управляет уведомлениями пользователей.
 * 
 * Endpoints:
 * - GET  /health                  - Health check
 * - POST /api/v1/notifications    - Отправить уведомление
 * - GET  /api/v1/notifications    - Получить уведомления (?userId=X)
 */
class NotificationApp : public BoostBeastApplication {
public:
    NotificationApp();
    ~NotificationApp() override;

protected:
    void loadEnvironment(int argc, char* argv[]) override;
    void configureInjection() override;
};
