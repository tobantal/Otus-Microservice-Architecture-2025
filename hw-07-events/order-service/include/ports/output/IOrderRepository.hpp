#pragma once

#include "domain/Order.hpp"
#include <optional>

namespace order::ports::output {

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

    virtual void save(const domain::Order& order) = 0;
    virtual std::optional<domain::Order> findById(const std::string& id) = 0;
};

} // namespace order::ports::output
