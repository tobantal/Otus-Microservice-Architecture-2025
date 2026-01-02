#pragma once

#include "ports/output/IAccountRepository.hpp"
#include "settings/DbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <iostream>

namespace billing::adapters::secondary {

class PostgresAccountRepository : public ports::output::IAccountRepository {
public:
    explicit PostgresAccountRepository(std::shared_ptr<settings::DbSettings> s) : settings_(s) {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec(R"(CREATE TABLE IF NOT EXISTS accounts (
            user_id VARCHAR(64) PRIMARY KEY, balance BIGINT DEFAULT 0
        ))");
        t.commit();
        std::cout << "[BillingRepo] Schema initialized" << std::endl;
    }
    
    void save(const domain::Account& a) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("INSERT INTO accounts (user_id, balance) VALUES ($1, $2) "
                      "ON CONFLICT (user_id) DO UPDATE SET balance = $2", a.userId, a.balance);
        t.commit();
    }
    
    void updateBalance(const std::string& userId, int64_t balance) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("UPDATE accounts SET balance = $1 WHERE user_id = $2", balance, userId);
        t.commit();
    }
    
    std::optional<domain::Account> findByUserId(const std::string& userId) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        auto r = t.exec_params("SELECT user_id, balance FROM accounts WHERE user_id = $1", userId);
        if (r.empty()) return std::nullopt;
        return domain::Account{r[0][0].as<std::string>(), r[0][1].as<int64_t>()};
    }

private:
    std::shared_ptr<settings::DbSettings> settings_;
};

} // namespace billing::adapters::secondary
