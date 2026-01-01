#pragma once

#include "domain/Notification.hpp"
#include <vector>

namespace notification::ports::output {

/**
 * @brief Репозиторий уведомлений
 */
class INotificationRepository {
public:
    virtual ~INotificationRepository() = default;

    virtual void save(const domain::Notification& notification) = 0;
    virtual std::vector<domain::Notification> findByUserId(const std::string& userId) = 0;
};

} // namespace notification::ports::output
