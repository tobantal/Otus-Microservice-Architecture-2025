#pragma once

#include <string>

namespace delivery::ports::output {

class IEventPublisher {
public:
    virtual ~IEventPublisher() = default;
    virtual void publish(const std::string& routingKey, const std::string& message) = 0;
};

} // namespace delivery::ports::output
