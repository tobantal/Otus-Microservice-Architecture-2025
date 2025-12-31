#pragma once

#include <IHttpHandler.hpp>
#include "ports/input/IAuthService.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace adapters::primary {

/**
 * @brief Обработчик выхода из системы
 * 
 * POST /api/v1/auth/logout
 * Headers: Authorization: Bearer <token>
 * Response: 200 {"message": "Logged out successfully"}
 */
class LogoutHandler : public IHttpHandler {
public:
    explicit LogoutHandler(std::shared_ptr<ports::input::IAuthService> authService)
        : authService_(std::move(authService))
    {
        std::cout << "[LogoutHandler] Created" << std::endl;
    }

    void handle(IRequest& req, IResponse& res) override {
        // Только POST
        if (req.getMethod() != "POST") {
            sendError(res, 405, "Method not allowed");
            return;
        }

        // Извлечение токена
        auto token = extractBearerToken(req);
        if (!token) {
            sendError(res, 401, "Authorization token required");
            return;
        }

        // Выход
        bool success = authService_->logout(*token);

        if (!success) {
            sendError(res, 401, "Invalid token");
            return;
        }

        // Успех
        nlohmann::json response = {
            {"message", "Logged out successfully"}
        };
        sendJson(res, 200, response);
    }

private:
    std::shared_ptr<ports::input::IAuthService> authService_;

    std::optional<std::string> extractBearerToken(IRequest& req) {
        auto headers = req.getHeaders();
        auto it = headers.find("Authorization");
        if (it == headers.end()) {
            return std::nullopt;
        }

        std::string auth = it->second;
        if (auth.substr(0, 7) != "Bearer ") {
            return std::nullopt;
        }

        return auth.substr(7);
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

} // namespace adapters::primary
