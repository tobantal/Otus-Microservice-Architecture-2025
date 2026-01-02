#pragma once

#include "ports/output/IEventPublisher.hpp"
#include "ports/output/IEventConsumer.hpp"
#include "settings/RabbitMQSettings.hpp"
#include <memory>
#include <string>

namespace warehouse::adapters::secondary {

class RabbitMQPublisher : public ports::output::IEventPublisher {
public:
    explicit RabbitMQPublisher(std::shared_ptr<settings::RabbitMQSettings> settings);
    ~RabbitMQPublisher();
    void publish(const std::string& routingKey, const std::string& message) override;
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class RabbitMQConsumer : public ports::output::IEventConsumer {
public:
    explicit RabbitMQConsumer(std::shared_ptr<settings::RabbitMQSettings> settings);
    ~RabbitMQConsumer();
    void subscribe(const std::vector<std::string>& routingKeys, ports::output::EventHandler handler) override;
    void start() override;
    void stop() override;
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace warehouse::adapters::secondary
