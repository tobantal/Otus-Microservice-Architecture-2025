#pragma once

#include "domain/Notification.hpp"
#include <vector>
#include <string>

namespace notification::ports::input {

/**
 * @brief Интерфейс сервиса уведомлений
 */
class INotificationService {
public:
    virtual ~INotificationService() = default;

    /**
     * @brief Отправить уведомление (сохранить)
     */
    virtual void send(const std::string& userId, const std::string& type, const std::string& message) = 0;

    /**
     * @brief Получить уведомления пользователя
     */
    virtual std::vector<domain::Notification> getByUserId(const std::string& userId) = 0;
};

} // namespace notification::ports::input
