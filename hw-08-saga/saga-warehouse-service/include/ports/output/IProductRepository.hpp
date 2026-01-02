#pragma once

#include "domain/Product.hpp"
#include <string>
#include <optional>

namespace warehouse::ports::output {

class IProductRepository {
public:
    virtual ~IProductRepository() = default;
    virtual void save(const domain::Product& p) = 0;
    virtual void updateStock(const std::string& productId, int32_t stock, int32_t reserved) = 0;
    virtual std::optional<domain::Product> findById(const std::string& productId) = 0;
};

} // namespace warehouse::ports::output
