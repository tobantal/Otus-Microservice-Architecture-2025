#pragma once

#include <string>
#include <cstdint>

namespace order::domain {

struct IdempotencyRecord {
    std::string key;
    int status;
    std::string body;
};

} // namespace order::domain
