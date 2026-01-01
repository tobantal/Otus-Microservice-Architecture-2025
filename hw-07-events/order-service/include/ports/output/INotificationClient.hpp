#pragma once

#include <string>

namespace order::ports::output {

/**
 * @brief Клиент к Notification Service
 */
class INotificationClient {
public:
    virtual ~INotificationClient() = default;

    /**
     * @brief Отправить уведомление пользователю
     */
    virtual void send(const std::string& userId, const std::string& type, const std::string& message) = 0;
};

} // namespace order::ports::output
