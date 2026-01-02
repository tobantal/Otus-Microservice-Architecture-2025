#pragma once

#include <string>
#include <cstdint>

namespace billing::domain {

struct Account {
    std::string userId;
    int64_t balance;
};

} // namespace billing::domain
