#pragma once

#include "ports/output/INotificationClient.hpp"
#include "ports/output/IServiceSettings.hpp"
#include <IHttpClient.hpp>
#include <SimpleRequest.hpp>
#include <SimpleResponse.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace order::adapters::secondary {

/**
 * @brief HTTP клиент к Notification Service
 * 
 * Использует IHttpClient для HTTP запросов.
 * Настройки получает через IServiceSettings (DI).
 */
class HttpNotificationClient : public ports::output::INotificationClient {
public:
    HttpNotificationClient(
        std::shared_ptr<IHttpClient> httpClient,
        std::shared_ptr<ports::output::IServiceSettings> settings
    ) : httpClient_(std::move(httpClient))
      , settings_(std::move(settings))
    {
        std::cout << "[HttpNotificationClient] Created, target: " 
                  << settings_->getHost() << ":" << settings_->getPort() << std::endl;
    }

    void send(const std::string& userId, const std::string& type, const std::string& message) override {
        try {
            nlohmann::json requestBody = {
                {"user_id", userId},
                {"type", type},
                {"message", message}
            };

            SimpleRequest request(
                "POST",
                "/api/v1/notifications",
                requestBody.dump(),
                settings_->getHost(),
                settings_->getPort(),
                {{"Content-Type", "application/json"}}
            );

            SimpleResponse response;
            httpClient_->send(request, response);

            std::cout << "[HttpNotificationClient] Notification sent to user: " << userId << std::endl;

        } catch (const std::exception& e) {
            // Логируем, но не падаем - уведомления не критичны
            std::cerr << "[HttpNotificationClient] Error: " << e.what() << std::endl;
        }
    }

private:
    std::shared_ptr<IHttpClient> httpClient_;
    std::shared_ptr<ports::output::IServiceSettings> settings_;
};

} // namespace order::adapters::secondary
