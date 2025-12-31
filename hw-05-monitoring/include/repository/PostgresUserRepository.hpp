#pragma once

#include "IUserRepository.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <mutex>
#include <iostream>

namespace repository {

/**
 * @brief PostgreSQL реализация репозитория пользователей
 */
class PostgresUserRepository : public IUserRepository {
public:
    PostgresUserRepository(
        const std::string& host,
        int port,
        const std::string& dbname,
        const std::string& user,
        const std::string& password
    );
    
    ~PostgresUserRepository() override;
    
    int64_t create(const domain::User& user) override;
    std::optional<domain::User> findById(int64_t id) override;
    std::vector<domain::User> findAll() override;
    bool update(int64_t id, const domain::User& user) override;
    bool deleteById(int64_t id) override;
    bool isConnected() const override;
    
private:
    domain::User rowToUser(const pqxx::row& row) const;
    
    std::unique_ptr<pqxx::connection> connection_;
    mutable std::mutex mutex_;
};

} // namespace repository
