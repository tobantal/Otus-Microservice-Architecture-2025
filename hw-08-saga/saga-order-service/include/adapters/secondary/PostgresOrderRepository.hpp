#pragma once

#include "ports/output/IOrderRepository.hpp"
#include "settings/DbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <iostream>

namespace order::adapters::secondary {

class PostgresOrderRepository : public ports::output::IOrderRepository {
public:
    explicit PostgresOrderRepository(std::shared_ptr<settings::DbSettings> s) : settings_(s) {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec(R"(CREATE TABLE IF NOT EXISTS orders (
            id VARCHAR(64) PRIMARY KEY, user_id VARCHAR(64), product_id VARCHAR(64),
            amount BIGINT, quantity INTEGER, state VARCHAR(32), failure_reason TEXT
        ))");
        t.commit();
        std::cout << "[OrderRepo] Schema initialized" << std::endl;
    }
    
    void save(const domain::Order& o) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("INSERT INTO orders VALUES ($1,$2,$3,$4,$5,$6,$7)",
            o.id, o.userId, o.productId, o.amount, o.quantity,
            domain::stateToString(o.state), o.failureReason);
        t.commit();
    }
    
    void updateState(const std::string& id, domain::SagaState state, const std::string& reason) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("UPDATE orders SET state=$1, failure_reason=$2 WHERE id=$3",
            domain::stateToString(state), reason, id);
        t.commit();
    }
    
    std::optional<domain::Order> findById(const std::string& id) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        auto r = t.exec_params("SELECT id,user_id,product_id,amount,quantity,state,failure_reason FROM orders WHERE id=$1", id);
        if (r.empty()) return std::nullopt;
        domain::Order o;
        o.id = r[0][0].as<std::string>();
        o.userId = r[0][1].as<std::string>();
        o.productId = r[0][2].as<std::string>();
        o.amount = r[0][3].as<int64_t>();
        o.quantity = r[0][4].as<int32_t>();
        o.state = domain::stringToState(r[0][5].as<std::string>());
        o.failureReason = r[0][6].is_null() ? "" : r[0][6].as<std::string>();
        return o;
    }

private:
    std::shared_ptr<settings::DbSettings> settings_;
};

} // namespace order::adapters::secondary
