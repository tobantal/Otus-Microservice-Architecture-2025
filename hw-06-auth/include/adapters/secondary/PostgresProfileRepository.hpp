#pragma once

#include "ports/output/IProfileRepository.hpp"
#include "ports/output/IDbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <iostream>

namespace adapters::secondary {

/**
 * @brief PostgreSQL реализация репозитория профилей
 */
class PostgresProfileRepository : public ports::output::IProfileRepository {
public:
    /**
     * @brief Конструктор с IDbSettings (для DI)
     */
    explicit PostgresProfileRepository(std::shared_ptr<ports::output::IDbSettings> settings)
        : settings_(std::move(settings))
    {
        std::string connStr = settings_->getConnectionString();

        try {
            connection_ = std::make_unique<pqxx::connection>(connStr);
            std::cout << "[PostgresProfileRepository] Connected to " 
                      << settings_->getHost() << ":" << settings_->getPort() 
                      << "/" << settings_->getDbName() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[PostgresProfileRepository] Connection failed: " << e.what() << std::endl;
            throw;
        }
    }

    ~PostgresProfileRepository() override {
        std::cout << "[PostgresProfileRepository] Destroyed" << std::endl;
    }

    void save(const domain::Profile& profile) override {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            pqxx::work txn(*connection_);
            
            txn.exec_params(
                "INSERT INTO profiles (user_id, first_name, last_name, email, phone, updated_at) "
                "VALUES ($1, $2, $3, $4, $5, NOW()) "
                "ON CONFLICT (user_id) DO UPDATE SET "
                "first_name = EXCLUDED.first_name, "
                "last_name = EXCLUDED.last_name, "
                "email = EXCLUDED.email, "
                "phone = EXCLUDED.phone, "
                "updated_at = NOW()",
                profile.userId,
                profile.firstName,
                profile.lastName,
                profile.email,
                profile.phone
            );
            
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "[PostgresProfileRepository] Save error: " << e.what() << std::endl;
            throw;
        }
    }

    std::optional<domain::Profile> findByUserId(const std::string& userId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            pqxx::work txn(*connection_);
            
            auto result = txn.exec_params(
                "SELECT user_id, first_name, last_name, email, phone, updated_at "
                "FROM profiles WHERE user_id = $1",
                userId
            );
            
            if (result.empty()) {
                return std::nullopt;
            }

            domain::Profile profile;
            profile.userId = result[0]["user_id"].as<std::string>();
            profile.firstName = result[0]["first_name"].is_null() ? "" : result[0]["first_name"].as<std::string>();
            profile.lastName = result[0]["last_name"].is_null() ? "" : result[0]["last_name"].as<std::string>();
            profile.email = result[0]["email"].is_null() ? "" : result[0]["email"].as<std::string>();
            profile.phone = result[0]["phone"].is_null() ? "" : result[0]["phone"].as<std::string>();
            profile.updatedAt = result[0]["updated_at"].is_null() ? "" : result[0]["updated_at"].as<std::string>();
            
            return profile;
        } catch (const std::exception& e) {
            std::cerr << "[PostgresProfileRepository] FindByUserId error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    void createEmpty(const std::string& userId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            pqxx::work txn(*connection_);
            
            txn.exec_params(
                "INSERT INTO profiles (user_id, updated_at) "
                "VALUES ($1, NOW()) "
                "ON CONFLICT (user_id) DO NOTHING",
                userId
            );
            
            txn.commit();
        } catch (const std::exception& e) {
            std::cerr << "[PostgresProfileRepository] CreateEmpty error: " << e.what() << std::endl;
            throw;
        }
    }

private:
    std::shared_ptr<ports::output::IDbSettings> settings_;
    std::unique_ptr<pqxx::connection> connection_;
    mutable std::mutex mutex_;
};

} // namespace adapters::secondary
