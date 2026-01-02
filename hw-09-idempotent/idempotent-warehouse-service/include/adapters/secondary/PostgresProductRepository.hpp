#pragma once

#include "ports/output/IProductRepository.hpp"
#include "settings/DbSettings.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <iostream>

namespace warehouse::adapters::secondary {

class PostgresProductRepository : public ports::output::IProductRepository {
public:
    explicit PostgresProductRepository(std::shared_ptr<settings::DbSettings> s) : settings_(s) {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec(R"(CREATE TABLE IF NOT EXISTS products (
            product_id VARCHAR(64) PRIMARY KEY, stock INTEGER DEFAULT 0, reserved INTEGER DEFAULT 0
        ))");
        t.commit();
        std::cout << "[WarehouseRepo] Schema initialized" << std::endl;
    }
    
    void save(const domain::Product& p) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("INSERT INTO products VALUES ($1,$2,$3) "
                      "ON CONFLICT (product_id) DO UPDATE SET stock=$2, reserved=$3",
                      p.productId, p.stock, p.reserved);
        t.commit();
    }
    
    void updateStock(const std::string& id, int32_t stock, int32_t reserved) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        t.exec_params("UPDATE products SET stock=$1, reserved=$2 WHERE product_id=$3", stock, reserved, id);
        t.commit();
    }
    
    std::optional<domain::Product> findById(const std::string& id) override {
        pqxx::connection c(settings_->getConnectionString());
        pqxx::work t(c);
        auto r = t.exec_params("SELECT product_id, stock, reserved FROM products WHERE product_id=$1", id);
        if (r.empty()) return std::nullopt;
        return domain::Product{r[0][0].as<std::string>(), r[0][1].as<int32_t>(), r[0][2].as<int32_t>()};
    }

private:
    std::shared_ptr<settings::DbSettings> settings_;
};

} // namespace warehouse::adapters::secondary
