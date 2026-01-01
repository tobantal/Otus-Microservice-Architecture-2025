#pragma once

#include <IHttpHandler.hpp>
#include "ports/input/INotificationService.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace notification::adapters::primary {

/**
 * @brief HTTP Handler для уведомлений
 * 
 * Endpoints:
 * - POST /api/v1/notifications          - отправить уведомление
 * - GET  /api/v1/notifications?userId=X - получить уведомления пользователя
 */
class NotificationHandler : public IHttpHandler {
public:
    explicit NotificationHandler(std::shared_ptr<ports::input::INotificationService> service)
        : service_(std::move(service))
    {
        std::cout << "[NotificationHandler] Created" << std::endl;
    }

    void handle(IRequest& req, IResponse& res) override {
        std::string method = req.getMethod();
        std::string path = req.getPath();

        if (method == "POST" && path == "/api/v1/notifications") {
            handleSend(req, res);
        } else if (method == "GET" && path == "/api/v1/notifications") {
            handleGet(req, res);
        } else {
            sendError(res, 404, "Not found");
        }
    }

private:
    std::shared_ptr<ports::input::INotificationService> service_;

    /**
     * @brief POST /api/v1/notifications
     * Body: {"user_id": "...", "type": "...", "message": "..."}
     */
    void handleSend(IRequest& req, IResponse& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.getBody());
        } catch (...) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        std::string userId = body.value("user_id", "");
        std::string type = body.value("type", "");
        std::string message = body.value("message", "");

        if (userId.empty() || type.empty() || message.empty()) {
            sendError(res, 400, "user_id, type, and message are required");
            return;
        }

        service_->send(userId, type, message);

        nlohmann::json response = {{"status", "sent"}};
        sendJson(res, 201, response);
    }

    /**
     * @brief GET /api/v1/notifications?userId=X
     */
    void handleGet(IRequest& req, IResponse& res) {
        auto params = req.getParams();
        auto it = params.find("userId");
        
        if (it == params.end() || it->second.empty()) {
            sendError(res, 400, "userId query parameter is required");
            return;
        }

        std::string userId = it->second;
        auto notifications = service_->getByUserId(userId);

        nlohmann::json response = nlohmann::json::array();
        for (const auto& n : notifications) {
            response.push_back(n.toJson());
        }

        sendJson(res, 200, response);
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

} // namespace notification::adapters::primary
