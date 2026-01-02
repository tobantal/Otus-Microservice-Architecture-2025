#pragma once

#include <string>
#include <functional>
#include <vector>

namespace billing::ports::output {

using EventHandler = std::function<void(const std::string& routingKey, const std::string& message)>;

class IEventConsumer {
public:
    virtual ~IEventConsumer() = default;
    virtual void subscribe(const std::vector<std::string>& routingKeys, EventHandler handler) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

} // namespace billing::ports::output
