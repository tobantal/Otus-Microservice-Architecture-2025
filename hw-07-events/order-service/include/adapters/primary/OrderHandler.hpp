#pragma once

#include <IHttpHandler.hpp>
#include "ports/input/IOrderService.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>
#include <regex>

namespace order::adapters::primary {

/**
 * @brief HTTP Handler для заказов
 * 
 * Endpoints:
 * - POST /api/v1/orders     - создать заказ
 * - GET  /api/v1/orders/{id} - получить заказ
 */
class OrderHandler : public IHttpHandler {
public:
    explicit OrderHandler(std::shared_ptr<ports::input::IOrderService> orderService)
        : orderService_(std::move(orderService))
    {
        std::cout << "[OrderHandler] Created" << std::endl;
    }

    void handle(IRequest& req, IResponse& res) override {
        std::string method = req.getMethod();
        std::string path = req.getPath();

        if (method == "POST" && path == "/api/v1/orders") {
            handleCreate(req, res);
        } else if (method == "GET" && path.find("/api/v1/orders/") == 0) {
            handleGet(req, res);
        } else {
            sendError(res, 404, "Not found");
        }
    }

private:
    std::shared_ptr<ports::input::IOrderService> orderService_;

    /**
     * @brief POST /api/v1/orders
     * Body: {"user_id": "...", "amount": 500}
     */
    void handleCreate(IRequest& req, IResponse& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.getBody());
        } catch (...) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        std::string userId = body.value("user_id", "");
        int64_t amount = body.value("amount", int64_t{0});

        if (userId.empty()) {
            sendError(res, 400, "user_id is required");
            return;
        }
        if (amount <= 0) {
            sendError(res, 400, "amount must be positive");
            return;
        }

        auto result = orderService_->createOrder(userId, amount);

        if (!result.success) {
            // 402 Payment Required для недостаточного баланса
            int status = (result.error.find("Insufficient") != std::string::npos) ? 402 : 400;
            nlohmann::json response = {
                {"error", result.error},
                {"order", result.order.toJson()}
            };
            sendJson(res, status, response);
            return;
        }

        sendJson(res, 201, result.order.toJson());
    }

    /**
     * @brief GET /api/v1/orders/{id}
     */
    void handleGet(IRequest& req, IResponse& res) {
        std::string path = req.getPath();
        
        std::regex pathRegex(R"(/api/v1/orders/([^/]+))");
        std::smatch match;
        
        if (!std::regex_match(path, match, pathRegex)) {
            sendError(res, 400, "Invalid path");
            return;
        }

        std::string orderId = match[1].str();
        auto order = orderService_->getOrder(orderId);

        if (!order) {
            sendError(res, 404, "Order not found");
            return;
        }

        sendJson(res, 200, order->toJson());
    }

    void sendJson(IResponse& res, int status, const nlohmann::json& body) {
        res.setStatus(status);
        res.setHeader("Content-Type", "application/json");
        res.setBody(body.dump());
    }

    void sendError(IResponse& res, int status, const std::string& message) {
        nlohmann::json body = {{"error", message}};
        sendJson(res, status, body);
    }
};

} // namespace order::adapters::primary
