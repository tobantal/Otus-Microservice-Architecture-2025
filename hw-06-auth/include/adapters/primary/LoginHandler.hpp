#pragma once

#include <IHttpHandler.hpp>
#include "ports/input/IAuthService.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace adapters::primary {

/**
 * @brief Обработчик входа в систему
 * 
 * POST /api/v1/auth/login
 * Body: {"username": "...", "password": "..."}
 * Response: 200 {"token": "...", "token_type": "Bearer", "expires_in": 86400}
 */
class LoginHandler : public IHttpHandler {
public:
    explicit LoginHandler(std::shared_ptr<ports::input::IAuthService> authService)
        : authService_(std::move(authService))
    {
        std::cout << "[LoginHandler] Created" << std::endl;
    }

    void handle(IRequest& req, IResponse& res) override {
        // Только POST
        if (req.getMethod() != "POST") {
            sendError(res, 405, "Method not allowed");
            return;
        }

        // Парсинг JSON
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.getBody());
        } catch (const std::exception& e) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        // Извлечение полей
        std::string username = body.value("username", "");
        std::string password = body.value("password", "");

        if (username.empty() || password.empty()) {
            sendError(res, 400, "Username and password are required");
            return;
        }

        // Вход
        auto result = authService_->login(username, password);

        if (!result.success) {
            sendError(res, 401, result.error);
            return;
        }

        // Успех
        nlohmann::json response = {
            {"token", result.token},
            {"token_type", result.tokenType},
            {"expires_in", result.expiresIn}
        };
        sendJson(res, 200, response);
    }

private:
    std::shared_ptr<ports::input::IAuthService> authService_;

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
