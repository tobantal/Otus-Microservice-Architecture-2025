#pragma once

#include <IHttpHandler.hpp>
#include <nlohmann/json.hpp>

namespace adapters::primary {

/**
 * @brief Обработчик health check
 * 
 * GET /health
 * Response: 200 {"status": "ok"}
 */
class HealthCheckHandler : public IHttpHandler {
public:
    void handle(IRequest& /*req*/, IResponse& res) override {
        nlohmann::json response = {{"status", "ok"}};
        
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(response.dump());
    }
};

} // namespace adapters::primary
