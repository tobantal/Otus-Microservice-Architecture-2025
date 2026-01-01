#pragma once

#include "ports/output/IOrderRepository.hpp"
#include "ports/output/IDbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <iostream>

namespace order::adapters::secondary {

/**
 * @brief PostgreSQL реализация репозитория заказов
 */
class PostgresOrderRepository : public ports::output::IOrderRepository {
public:
    explicit PostgresOrderRepository(std::shared_ptr<ports::output::IDbSettings> settings)
        : settings_(std::move(settings))
    {
        std::string connStr = settings_->getConnectionString();
        
        try {
            connection_ = std::make_unique<pqxx::connection>(connStr);
            std::cout << "[PostgresOrderRepository] Connected to " 
                      << settings_->getHost() << ":" << settings_->getPort() 
                      << "/" << settings_->getDbName() << std::endl;
            
            initSchema();
        } catch (const std::exception& e) {
            std::cerr << "[PostgresOrderRepository] Connection failed: " << e.what() << std::endl;
            throw;
        }
    }

    void save(const domain::Order& order) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        pqxx::work txn(*connection_);
        
        std::string statusStr = domain::orderStatusToString(order.status);
        
        txn.exec_params(
            "INSERT INTO orders (id, user_id, amount, status, created_at) "
            "VALUES ($1, $2, $3, $4, $5) "
            "ON CONFLICT (id) DO UPDATE SET status = $4",
            order.id,
            order.userId,
            order.amount,
            statusStr,
            order.createdAt
        );
        
        txn.commit();
    }

    std::optional<domain::Order> findById(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            "SELECT id, user_id, amount, status, created_at FROM orders WHERE id = $1",
            id
        );
        
        txn.commit();
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        domain::Order order;
        order.id = result[0]["id"].as<std::string>();
        order.userId = result[0]["user_id"].as<std::string>();
        order.amount = result[0]["amount"].as<int64_t>();
        order.createdAt = result[0]["created_at"].as<std::string>();
        
        std::string statusStr = result[0]["status"].as<std::string>();
        if (statusStr == "paid") {
            order.status = domain::OrderStatus::PAID;
        } else if (statusStr == "failed") {
            order.status = domain::OrderStatus::FAILED;
        } else {
            order.status = domain::OrderStatus::CREATED;
        }
        
        return order;
    }

private:
    std::shared_ptr<ports::output::IDbSettings> settings_;
    std::unique_ptr<pqxx::connection> connection_;
    mutable std::mutex mutex_;

    void initSchema() {
        pqxx::work txn(*connection_);
        
        txn.exec(R"(
            CREATE TABLE IF NOT EXISTS orders (
                id VARCHAR(255) PRIMARY KEY,
                user_id VARCHAR(255) NOT NULL,
                amount BIGINT NOT NULL,
                status VARCHAR(50) NOT NULL,
                created_at VARCHAR(50) NOT NULL
            )
        )");
        
        txn.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_orders_user_id 
            ON orders(user_id)
        )");
        
        txn.commit();
        std::cout << "[PostgresOrderRepository] Schema initialized" << std::endl;
    }
};

} // namespace order::adapters::secondary
