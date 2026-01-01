#pragma once

#include <IHttpHandler.hpp>
#include <nlohmann/json.hpp>

namespace billing::adapters::primary {

/**
 * @brief Health check handler
 */
class HealthHandler : public IHttpHandler {
public:
    void handle(IRequest& /*req*/, IResponse& res) override {
        nlohmann::json response = {{"status", "ok"}};
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(response.dump());
    }
};

} // namespace billing::adapters::primary
