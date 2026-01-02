#pragma once

#include <IHttpHandler.hpp>
#include <IRequest.hpp>
#include <IResponse.hpp>

namespace order::adapters::primary {

class HealthHandler : public IHttpHandler {
public:
    void handle(IRequest& req, IResponse& res) override {
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(R"({"status":"healthy","service":"saga-order-service"})");
    }
};

} // namespace order::adapters::primary
