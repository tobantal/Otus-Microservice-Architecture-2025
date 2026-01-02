#pragma once

#include <string>

namespace order::ports::output {

class IEventPublisher {
public:
    virtual ~IEventPublisher() = default;
    virtual void publish(const std::string& routingKey, const std::string& message) = 0;
};

} // namespace order::ports::output
