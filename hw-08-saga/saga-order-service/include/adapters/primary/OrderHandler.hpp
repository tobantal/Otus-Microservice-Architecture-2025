#pragma once

#include <IHttpHandler.hpp>
#include <IRequest.hpp>
#include <IResponse.hpp>
#include <RouteMatcher.hpp>
#include "ports/input/IOrderService.hpp"
#include "domain/Order.hpp"
#include <nlohmann/json.hpp>
#include <memory>

namespace order::adapters::primary {

class OrderHandler : public IHttpHandler {
public:
    explicit OrderHandler(std::shared_ptr<ports::input::IOrderService> svc) : svc_(svc) {}
    
    void handle(IRequest& req, IResponse& res) override {
        try {
            if (req.getMethod() == "POST" && req.getPath() == "/api/v1/orders") {
                auto body = nlohmann::json::parse(req.getBody());
                auto id = svc_->createOrder(body["user_id"], body["product_id"],
                                           body["amount"], body.value("quantity", 1));
                res.setStatus(202);
                res.setHeader("Content-Type", "application/json");
                res.setBody(nlohmann::json{{"order_id", id}, {"status", "BILLING_PENDING"}}.dump());
            }
            else if (req.getMethod() == "GET" && RouteMatcher::matches("/api/v1/orders/*", req.getPath())) {
                auto id = req.getPath().substr(std::string("/api/v1/orders/").length());
                auto order = svc_->getOrder(id);
                if (!order) {
                    res.setStatus(404);
                    res.setBody(R"({"error":"Not found"})");
                } else {
                    res.setStatus(200);
                    res.setBody(nlohmann::json{
                        {"id", order->id}, {"user_id", order->userId},
                        {"state", domain::stateToString(order->state)}
                    }.dump());
                }
                res.setHeader("Content-Type", "application/json");
            }
            else {
                res.setStatus(404);
                res.setHeader("Content-Type", "application/json");
                res.setBody(R"({"error":"Not found"})");
            }
        } catch (const std::exception& e) {
            res.setStatus(500);
            res.setHeader("Content-Type", "application/json");
            res.setBody(nlohmann::json{{"error", e.what()}}.dump());
        }
    }

private:
    std::shared_ptr<ports::input::IOrderService> svc_;
};

} // namespace order::adapters::primary
