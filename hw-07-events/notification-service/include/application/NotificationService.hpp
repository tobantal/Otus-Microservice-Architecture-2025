#pragma once

#include "ports/input/INotificationService.hpp"
#include "ports/output/INotificationRepository.hpp"
#include <memory>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace notification::application {

/**
 * @brief Реализация сервиса уведомлений
 */
class NotificationService : public ports::input::INotificationService {
public:
    explicit NotificationService(std::shared_ptr<ports::output::INotificationRepository> repository)
        : repository_(std::move(repository))
    {
        std::cout << "[NotificationService] Created" << std::endl;
    }

    void send(const std::string& userId, const std::string& type, const std::string& message) override {
        domain::Notification notification;
        notification.id = generateId();
        notification.userId = userId;
        notification.type = type;
        notification.message = message;
        notification.createdAt = getCurrentTimestamp();

        repository_->save(notification);

        std::cout << "[NotificationService] Notification sent to user: " << userId 
                  << ", type: " << type << std::endl;
    }

    std::vector<domain::Notification> getByUserId(const std::string& userId) override {
        return repository_->findByUserId(userId);
    }

private:
    std::shared_ptr<ports::output::INotificationRepository> repository_;

    std::string generateId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(8) << (dis(gen) & 0xFFFFFFFF);
        ss << "-";
        ss << std::setw(4) << (dis(gen) & 0xFFFF);
        ss << "-4";  // Version 4
        ss << std::setw(3) << (dis(gen) & 0xFFF);
        ss << "-";
        ss << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000);
        ss << "-";
        ss << std::setw(12) << (dis(gen) & 0xFFFFFFFFFFFF);
        return ss.str();
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

} // namespace notification::application
