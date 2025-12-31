#pragma once

#include <IHttpHandler.hpp>
#include "ports/input/IAuthService.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace adapters::primary {

/**
 * @brief Обработчик регистрации пользователей
 * 
 * POST /api/v1/auth/register
 * Body: {"username": "...", "password": "..."}
 * Response: 201 {"user_id": "...", "message": "..."}
 */
class RegisterHandler : public IHttpHandler {
public:
    explicit RegisterHandler(std::shared_ptr<ports::input::IAuthService> authService)
        : authService_(std::move(authService))
    {
        std::cout << "[RegisterHandler] Created" << std::endl;
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

        if (username.empty()) {
            sendError(res, 400, "Username is required");
            return;
        }

        if (password.empty()) {
            sendError(res, 400, "Password is required");
            return;
        }

        // Регистрация
        auto result = authService_->registerUser(username, password);

        if (!result.success) {
            // Определяем код ошибки
            int status = 400;
            if (result.error.find("already exists") != std::string::npos) {
                status = 409; // Conflict
            }
            sendError(res, status, result.error);
            return;
        }

        // Успех
        nlohmann::json response = {
            {"user_id", result.userId},
            {"message", "User registered successfully"}
        };
        sendJson(res, 201, response);
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
