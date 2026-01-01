#pragma once

#include "ports/output/IBillingClient.hpp"
#include "ports/output/IServiceSettings.hpp"
#include <IHttpClient.hpp>
#include <SimpleRequest.hpp>
#include <SimpleResponse.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace order::adapters::secondary {

/**
 * @brief HTTP клиент к Billing Service
 * 
 * Использует IHttpClient для HTTP запросов.
 * Настройки получает через IServiceSettings (DI).
 */
class HttpBillingClient : public ports::output::IBillingClient {
public:
    HttpBillingClient(
        std::shared_ptr<IHttpClient> httpClient,
        std::shared_ptr<ports::output::IServiceSettings> settings
    ) : httpClient_(std::move(httpClient))
      , settings_(std::move(settings))
    {
        std::cout << "[HttpBillingClient] Created, target: " 
                  << settings_->getHost() << ":" << settings_->getPort() << std::endl;
    }

    ports::output::BillingResponse charge(const std::string& userId, int64_t amount) override {
        try {
            nlohmann::json requestBody = {
                {"user_id", userId},
                {"amount", amount}
            };

            SimpleRequest request(
                "POST",
                "/api/v1/billing/charge",
                requestBody.dump(),
                settings_->getHost(),
                settings_->getPort(),
                {{"Content-Type", "application/json"}}
            );

            SimpleResponse response;
            bool success = httpClient_->send(request, response);

            if (!success) {
                return {false, "Failed to connect to Billing service", 0};
            }

            auto json = nlohmann::json::parse(response.getBody());

            if (json.contains("error")) {
                return {false, json["error"].get<std::string>(), 0};
            }

            return {
                true, 
                "", 
                json.value("balance", int64_t{0})
            };

        } catch (const std::exception& e) {
            std::cerr << "[HttpBillingClient] Error: " << e.what() << std::endl;
            return {false, std::string("Billing service error: ") + e.what(), 0};
        }
    }

private:
    std::shared_ptr<IHttpClient> httpClient_;
    std::shared_ptr<ports::output::IServiceSettings> settings_;
};

} // namespace order::adapters::secondary
