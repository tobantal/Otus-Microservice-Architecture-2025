#include "adapters/secondary/RabbitMQAdapter.hpp"

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include <boost/asio.hpp>

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

namespace delivery::adapters::secondary {

// =============================================================================
// RabbitMQPublisher::Impl - как в MVP
// =============================================================================
class RabbitMQPublisher::Impl {
public:
    Impl(std::shared_ptr<settings::RabbitMQSettings> settings) 
        : settings_(settings), running_(false) {
        start();
    }
    
    ~Impl() {
        stop();
    }
    
    void publish(const std::string& routingKey, const std::string& message) {
        if (!running_ || !channel_) {
            std::cerr << "[Publisher] Not connected, cannot publish" << std::endl;
            return;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        channel_->publish("saga.events", routingKey, message);
        std::cout << "[Publisher] " << routingKey << std::endl;
    }
    
private:
    std::shared_ptr<settings::RabbitMQSettings> settings_;
    
    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<AMQP::LibBoostAsioHandler> handler_;
    std::unique_ptr<AMQP::TcpConnection> connection_;
    std::unique_ptr<AMQP::TcpChannel> channel_;
    
    std::mutex mutex_;
    std::atomic<bool> running_;
    std::thread ioThread_;
    
    void start() {
        if (running_) return;
        
        running_ = true;
        
        ioThread_ = std::thread([this]() {
            try {
                ioContext_ = std::make_unique<boost::asio::io_context>();
                handler_ = std::make_unique<AMQP::LibBoostAsioHandler>(*ioContext_);
                
                std::string connStr = "amqp://" 
                    + settings_->getUser() + ":" + settings_->getPassword() 
                    + "@" + settings_->getHost() + ":" + std::to_string(settings_->getPort()) + "/";
                
                connection_ = std::make_unique<AMQP::TcpConnection>(
                    handler_.get(), AMQP::Address(connStr));
                
                channel_ = std::make_unique<AMQP::TcpChannel>(connection_.get());
                
                channel_->declareExchange("saga.events", AMQP::topic, AMQP::durable)
                    .onSuccess([]() {
                        std::cout << "[Publisher] Exchange declared" << std::endl;
                    })
                    .onError([](const char* msg) {
                        std::cerr << "[Publisher] Exchange error: " << msg << std::endl;
                    });
                
                std::cout << "[Publisher] Connected to RabbitMQ" << std::endl;
                
                ioContext_->run();
                
            } catch (const std::exception& e) {
                std::cerr << "[Publisher] Error: " << e.what() << std::endl;
                running_ = false;
            }
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    void stop() {
        if (!running_) return;
        running_ = false;
        
        if (ioContext_) {
            ioContext_->stop();
        }
        if (ioThread_.joinable()) {
            ioThread_.join();
        }
        
        channel_.reset();
        connection_.reset();
        handler_.reset();
        ioContext_.reset();
    }
};

RabbitMQPublisher::RabbitMQPublisher(std::shared_ptr<settings::RabbitMQSettings> s)
    : impl_(std::make_unique<Impl>(s)) {}
RabbitMQPublisher::~RabbitMQPublisher() = default;
void RabbitMQPublisher::publish(const std::string& k, const std::string& m) { impl_->publish(k, m); }

// =============================================================================
// RabbitMQConsumer::Impl - как в MVP
// =============================================================================
class RabbitMQConsumer::Impl {
public:
    Impl(std::shared_ptr<settings::RabbitMQSettings> settings) 
        : settings_(settings), running_(false) {}
    
    ~Impl() { stop(); }
    
    void subscribe(const std::vector<std::string>& keys, ports::output::EventHandler h) {
        std::lock_guard<std::mutex> lock(mutex_);
        routingKeys_ = keys;
        handler_ = h;
    }
    
    void start() {
        if (running_) return;
        
        running_ = true;
        
        ioThread_ = std::thread([this]() {
            try {
                ioContext_ = std::make_unique<boost::asio::io_context>();
                amqpHandler_ = std::make_unique<AMQP::LibBoostAsioHandler>(*ioContext_);
                
                std::string connStr = "amqp://" 
                    + settings_->getUser() + ":" + settings_->getPassword() 
                    + "@" + settings_->getHost() + ":" + std::to_string(settings_->getPort()) + "/";
                
                connection_ = std::make_unique<AMQP::TcpConnection>(
                    amqpHandler_.get(), AMQP::Address(connStr));
                
                channel_ = std::make_unique<AMQP::TcpChannel>(connection_.get());
                
                // Declare queue
                std::string queueName = "saga.delivery.queue";
                channel_->declareQueue(queueName, AMQP::durable)
                    .onSuccess([this, queueName](const std::string& name, uint32_t, uint32_t) {
                        std::cout << "[Consumer] Queue declared: " << name << std::endl;
                        
                        // Bind to routing keys
                        for (const auto& key : routingKeys_) {
                            channel_->bindQueue("saga.events", name, key)
                                .onSuccess([key]() {
                                    std::cout << "[Consumer] Bound to: " << key << std::endl;
                                });
                        }
                        
                        // Start consuming
                        channel_->consume(name)
                            .onReceived([this](const AMQP::Message& msg, uint64_t tag, bool) {
                                std::string body(msg.body(), msg.bodySize());
                                std::string routingKey = msg.routingkey();
                                
                                std::cout << "[Consumer] Received: " << routingKey << std::endl;
                                
                                if (handler_) {
                                    try {
                                        handler_(routingKey, body);
                                    } catch (const std::exception& e) {
                                        std::cerr << "[Consumer] Handler error: " << e.what() << std::endl;
                                    }
                                }
                                
                                channel_->ack(tag);
                            })
                            .onError([](const char* msg) {
                                std::cerr << "[Consumer] Consume error: " << msg << std::endl;
                            });
                    })
                    .onError([](const char* msg) {
                        std::cerr << "[Consumer] Queue error: " << msg << std::endl;
                    });
                
                std::cout << "[Consumer] Connected to RabbitMQ" << std::endl;
                
                ioContext_->run();
                
            } catch (const std::exception& e) {
                std::cerr << "[Consumer] Error: " << e.what() << std::endl;
                running_ = false;
            }
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    void stop() {
        if (!running_) return;
        running_ = false;
        
        if (ioContext_) {
            ioContext_->stop();
        }
        if (ioThread_.joinable()) {
            ioThread_.join();
        }
        
        channel_.reset();
        connection_.reset();
        amqpHandler_.reset();
        ioContext_.reset();
    }
    
private:
    std::shared_ptr<settings::RabbitMQSettings> settings_;
    std::vector<std::string> routingKeys_;
    ports::output::EventHandler handler_;
    
    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<AMQP::LibBoostAsioHandler> amqpHandler_;
    std::unique_ptr<AMQP::TcpConnection> connection_;
    std::unique_ptr<AMQP::TcpChannel> channel_;
    
    std::mutex mutex_;
    std::atomic<bool> running_;
    std::thread ioThread_;
};

RabbitMQConsumer::RabbitMQConsumer(std::shared_ptr<settings::RabbitMQSettings> s)
    : impl_(std::make_unique<Impl>(s)) {}
RabbitMQConsumer::~RabbitMQConsumer() = default;
void RabbitMQConsumer::subscribe(const std::vector<std::string>& k, ports::output::EventHandler h) { impl_->subscribe(k, h); }
void RabbitMQConsumer::start() { impl_->start(); }
void RabbitMQConsumer::stop() { impl_->stop(); }

} // namespace delivery::adapters::secondary
