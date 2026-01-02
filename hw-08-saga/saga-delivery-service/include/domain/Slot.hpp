#pragma once

#include <string>

namespace delivery::domain {

struct Slot {
    std::string id;
    std::string slotTime;
    std::string orderId;  // empty if available
    std::string status;   // available, booked
};

} // namespace delivery::domain
