#pragma once

#include "ports/output/INotificationRepository.hpp"
#include "ports/output/IDbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <iostream>

namespace notification::adapters::secondary {

/**
 * @brief PostgreSQL реализация репозитория уведомлений
 */
class PostgresNotificationRepository : public ports::output::INotificationRepository {
public:
    explicit PostgresNotificationRepository(std::shared_ptr<ports::output::IDbSettings> settings)
        : settings_(std::move(settings))
    {
        std::string connStr = settings_->getConnectionString();
        
        try {
            connection_ = std::make_unique<pqxx::connection>(connStr);
            std::cout << "[PostgresNotificationRepository] Connected to " 
                      << settings_->getHost() << ":" << settings_->getPort() 
                      << "/" << settings_->getDbName() << std::endl;
            
            initSchema();
        } catch (const std::exception& e) {
            std::cerr << "[PostgresNotificationRepository] Connection failed: " << e.what() << std::endl;
            throw;
        }
    }

    void save(const domain::Notification& notification) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        pqxx::work txn(*connection_);
        
        txn.exec_params(
            "INSERT INTO notifications (id, user_id, type, message, created_at) "
            "VALUES ($1, $2, $3, $4, $5)",
            notification.id,
            notification.userId,
            notification.type,
            notification.message,
            notification.createdAt
        );
        
        txn.commit();
    }

    std::vector<domain::Notification> findByUserId(const std::string& userId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            "SELECT id, user_id, type, message, created_at "
            "FROM notifications WHERE user_id = $1 ORDER BY created_at ASC",
            userId
        );
        
        txn.commit();
        
        std::vector<domain::Notification> notifications;
        notifications.reserve(result.size());
        
        for (const auto& row : result) {
            domain::Notification n;
            n.id = row["id"].as<std::string>();
            n.userId = row["user_id"].as<std::string>();
            n.type = row["type"].as<std::string>();
            n.message = row["message"].as<std::string>();
            n.createdAt = row["created_at"].as<std::string>();
            notifications.push_back(std::move(n));
        }
        
        return notifications;
    }

private:
    std::shared_ptr<ports::output::IDbSettings> settings_;
    std::unique_ptr<pqxx::connection> connection_;
    mutable std::mutex mutex_;

    void initSchema() {
        pqxx::work txn(*connection_);
        
        txn.exec(R"(
            CREATE TABLE IF NOT EXISTS notifications (
                id VARCHAR(255) PRIMARY KEY,
                user_id VARCHAR(255) NOT NULL,
                type VARCHAR(100) NOT NULL,
                message TEXT NOT NULL,
                created_at VARCHAR(50) NOT NULL
            )
        )");
        
        txn.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_notifications_user_id 
            ON notifications(user_id)
        )");
        
        txn.commit();
        std::cout << "[PostgresNotificationRepository] Schema initialized" << std::endl;
    }
};

} // namespace notification::adapters::secondary
