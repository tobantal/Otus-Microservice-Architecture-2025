#pragma once

#include "domain/Order.hpp"
#include <string>
#include <optional>

namespace order::ports::output {

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual void save(const domain::Order& order) = 0;
    virtual void updateState(const std::string& id, domain::SagaState state, const std::string& reason = "") = 0;
    virtual std::optional<domain::Order> findById(const std::string& id) = 0;
};

} // namespace order::ports::output
