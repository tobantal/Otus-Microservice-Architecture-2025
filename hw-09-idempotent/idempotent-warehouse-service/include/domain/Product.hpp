#pragma once

#include <string>
#include <cstdint>

namespace warehouse::domain {

struct Product {
    std::string productId;
    int32_t stock;
    int32_t reserved;
};

} // namespace warehouse::domain
