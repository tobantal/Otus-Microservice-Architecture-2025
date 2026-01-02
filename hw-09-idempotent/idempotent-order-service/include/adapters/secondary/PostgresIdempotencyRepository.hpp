#pragma once

#include "ports/output/IIdempotencyRepository.hpp"
#include "settings/DbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <iostream>

namespace order::adapters::secondary {

class PostgresIdempotencyRepository : public ports::output::IIdempotencyRepository {
public:
    explicit PostgresIdempotencyRepository(std::shared_ptr<settings::DbSettings> s) : settings_(s) {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec(R"(CREATE TABLE IF NOT EXISTS idempotency_keys (
            key VARCHAR(64) PRIMARY KEY,
            response_status INTEGER,
            response_body TEXT,
            created_at TIMESTAMP DEFAULT NOW()
        ))");
        t.commit();
        std::cout << "[IdempotencyRepo] Schema initialized" << std::endl;
    }
    
    std::optional<domain::IdempotencyRecord> find(const std::string& key) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        auto r = t.exec_params(
            "SELECT key, response_status, response_body FROM idempotency_keys WHERE key=$1", key);
        if (r.empty()) return std::nullopt;
        return domain::IdempotencyRecord{
            r[0][0].as<std::string>(),
            r[0][1].as<int>(),
            r[0][2].as<std::string>()
        };
    }
    
    void save(const std::string& key, int status, const std::string& body) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params(
            "INSERT INTO idempotency_keys (key, response_status, response_body) VALUES ($1, $2, $3) "
            "ON CONFLICT (key) DO NOTHING",
            key, status, body);
        t.commit();
        std::cout << "[IdempotencyRepo] Saved key: " << key << std::endl;
    }

private:
    std::shared_ptr<settings::DbSettings> settings_;
};

} // namespace order::adapters::secondary
