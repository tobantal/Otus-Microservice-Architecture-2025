#pragma once

#include "domain/Order.hpp"
#include <string>
#include <optional>

namespace order::ports::input {

class IOrderService {
public:
    virtual ~IOrderService() = default;
    virtual std::string createOrder(const std::string& userId, const std::string& productId,
                                    int64_t amount, int32_t quantity) = 0;
    virtual std::optional<domain::Order> getOrder(const std::string& orderId) = 0;
};

} // namespace order::ports::input
