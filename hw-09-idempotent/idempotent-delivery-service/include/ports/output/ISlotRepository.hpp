#pragma once

#include "domain/Slot.hpp"
#include <string>
#include <optional>

namespace delivery::ports::output {

class ISlotRepository {
public:
    virtual ~ISlotRepository() = default;
    virtual void save(const domain::Slot& slot) = 0;
    virtual void book(const std::string& slotId, const std::string& orderId) = 0;
    virtual void release(const std::string& orderId) = 0;
    virtual std::optional<domain::Slot> findAvailable() = 0;
    virtual void deleteAll() = 0;  // For tests
};

} // namespace delivery::ports::output
