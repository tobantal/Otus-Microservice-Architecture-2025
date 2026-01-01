#pragma once

#include "ports/output/IBillingRepository.hpp"
#include "ports/output/IDbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <iostream>

namespace billing::adapters::secondary {

/**
 * @brief PostgreSQL реализация репозитория биллинга
 */
class PostgresBillingRepository : public ports::output::IBillingRepository {
public:
    explicit PostgresBillingRepository(std::shared_ptr<ports::output::IDbSettings> settings)
        : settings_(std::move(settings))
    {
        std::string connStr = settings_->getConnectionString();
        
        try {
            connection_ = std::make_unique<pqxx::connection>(connStr);
            std::cout << "[PostgresBillingRepository] Connected to " 
                      << settings_->getHost() << ":" << settings_->getPort() 
                      << "/" << settings_->getDbName() << std::endl;
            
            initSchema();
        } catch (const std::exception& e) {
            std::cerr << "[PostgresBillingRepository] Connection failed: " << e.what() << std::endl;
            throw;
        }
    }

    void save(const domain::BillingAccount& account) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        pqxx::work txn(*connection_);
        
        txn.exec_params(
            "INSERT INTO billing_accounts (user_id, balance) VALUES ($1, $2) "
            "ON CONFLICT (user_id) DO UPDATE SET balance = $2",
            account.userId,
            account.balance
        );
        
        txn.commit();
    }

    std::optional<domain::BillingAccount> findByUserId(const std::string& userId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            "SELECT user_id, balance FROM billing_accounts WHERE user_id = $1",
            userId
        );
        
        txn.commit();
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        domain::BillingAccount account;
        account.userId = result[0]["user_id"].as<std::string>();
        account.balance = result[0]["balance"].as<int64_t>();
        
        return account;
    }

    bool exists(const std::string& userId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            "SELECT 1 FROM billing_accounts WHERE user_id = $1",
            userId
        );
        
        txn.commit();
        
        return !result.empty();
    }

private:
    std::shared_ptr<ports::output::IDbSettings> settings_;
    std::unique_ptr<pqxx::connection> connection_;
    mutable std::mutex mutex_;

    void initSchema() {
        pqxx::work txn(*connection_);
        
        txn.exec(R"(
            CREATE TABLE IF NOT EXISTS billing_accounts (
                user_id VARCHAR(255) PRIMARY KEY,
                balance BIGINT NOT NULL DEFAULT 0
            )
        )");
        
        txn.commit();
        std::cout << "[PostgresBillingRepository] Schema initialized" << std::endl;
    }
};

} // namespace billing::adapters::secondary
