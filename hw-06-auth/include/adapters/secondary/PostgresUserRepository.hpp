#pragma once

#include "ports/output/IUserRepository.hpp"
#include "ports/output/IDbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <iostream>

namespace adapters::secondary {

/**
 * @brief PostgreSQL реализация репозитория пользователей
 */
class PostgresUserRepository : public ports::output::IUserRepository {
public:
    /**
     * @brief Конструктор с IDbSettings (для DI)
     */
    explicit PostgresUserRepository(std::shared_ptr<ports::output::IDbSettings> settings)
        : settings_(std::move(settings))
    {
        std::string connStr = settings_->getConnectionString();
        
        try {
            connection_ = std::make_unique<pqxx::connection>(connStr);
            std::cout << "[PostgresUserRepository] Connected to " 
                      << settings_->getHost() << ":" << settings_->getPort() 
                      << "/" << settings_->getDbName() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[PostgresUserRepository] Connection failed: " << e.what() << std::endl;
            throw;
        }
    }

    ~PostgresUserRepository() override {
        std::cout << "[PostgresUserRepository] Destroyed" << std::endl;
    }

    void save(const domain::User& user) override {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            pqxx::work txn(*connection_);
            
            txn.exec_params(
                "INSERT INTO users (id, username, password_hash, created_at) "
                "VALUES ($1, $2, $3, NOW()) "
                "ON CONFLICT (id) DO UPDATE SET "
                "username = EXCLUDED.username, "
                "password_hash = EXCLUDED.password_hash",
                user.id,
                user.username,
                user.passwordHash
            );
            
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "[PostgresUserRepository] Save error: " << e.what() << std::endl;
            throw;
        }
    }

    std::optional<domain::User> findById(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            pqxx::work txn(*connection_);
            
            auto result = txn.exec_params(
                "SELECT id, username, password_hash, created_at "
                "FROM users WHERE id = $1",
                id
            );
            
            if (result.empty()) {
                return std::nullopt;
            }

            domain::User user;
            user.id = result[0]["id"].as<std::string>();
            user.username = result[0]["username"].as<std::string>();
            user.passwordHash = result[0]["password_hash"].as<std::string>();
            user.createdAt = result[0]["created_at"].as<std::string>();
            
            return user;
        } catch (const std::exception& e) {
            std::cerr << "[PostgresUserRepository] FindById error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<domain::User> findByUsername(const std::string& username) override {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            pqxx::work txn(*connection_);
            
            auto result = txn.exec_params(
                "SELECT id, username, password_hash, created_at "
                "FROM users WHERE username = $1",
                username
            );
            
            if (result.empty()) {
                return std::nullopt;
            }

            domain::User user;
            user.id = result[0]["id"].as<std::string>();
            user.username = result[0]["username"].as<std::string>();
            user.passwordHash = result[0]["password_hash"].as<std::string>();
            user.createdAt = result[0]["created_at"].as<std::string>();
            
            return user;
        } catch (const std::exception& e) {
            std::cerr << "[PostgresUserRepository] FindByUsername error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    bool existsByUsername(const std::string& username) override {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            pqxx::work txn(*connection_);
            
            auto result = txn.exec_params(
                "SELECT 1 FROM users WHERE username = $1",
                username
            );
            
            return !result.empty();
        } catch (const std::exception& e) {
            std::cerr << "[PostgresUserRepository] ExistsByUsername error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    std::shared_ptr<ports::output::IDbSettings> settings_;
    std::unique_ptr<pqxx::connection> connection_;
    mutable std::mutex mutex_;
};

} // namespace adapters::secondary
