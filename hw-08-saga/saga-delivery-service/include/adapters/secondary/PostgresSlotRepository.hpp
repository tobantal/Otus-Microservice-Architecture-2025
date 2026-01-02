#pragma once

#include "ports/output/ISlotRepository.hpp"
#include "settings/DbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <iostream>

namespace delivery::adapters::secondary {

class PostgresSlotRepository : public ports::output::ISlotRepository {
public:
    explicit PostgresSlotRepository(std::shared_ptr<settings::DbSettings> s) : settings_(s) {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec(R"(CREATE TABLE IF NOT EXISTS slots (
            id VARCHAR(64) PRIMARY KEY, slot_time VARCHAR(64),
            order_id VARCHAR(64), status VARCHAR(32) DEFAULT 'available'
        ))");
        t.commit();
        std::cout << "[DeliveryRepo] Schema initialized" << std::endl;
    }
    
    void save(const domain::Slot& s) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("INSERT INTO slots VALUES ($1,$2,$3,$4)", s.id, s.slotTime, s.orderId, s.status);
        t.commit();
    }
    
    void book(const std::string& slotId, const std::string& orderId) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("UPDATE slots SET order_id=$1, status='booked' WHERE id=$2", orderId, slotId);
        t.commit();
    }
    
    void release(const std::string& orderId) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("UPDATE slots SET order_id='', status='available' WHERE order_id=$1", orderId);
        t.commit();
    }
    
    std::optional<domain::Slot> findAvailable() override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        auto r = t.exec("SELECT id, slot_time, order_id, status FROM slots WHERE status='available' LIMIT 1");
        if (r.empty()) return std::nullopt;
        return domain::Slot{
            r[0][0].as<std::string>(), r[0][1].as<std::string>(),
            r[0][2].is_null() ? "" : r[0][2].as<std::string>(),
            r[0][3].as<std::string>()
        };
    }
    
    void deleteAll() override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec("DELETE FROM slots");
        t.commit();
        std::cout << "[DeliveryRepo] All slots deleted" << std::endl;
    }

private:
    std::shared_ptr<settings::DbSettings> settings_;
};

} // namespace delivery::adapters::secondary
