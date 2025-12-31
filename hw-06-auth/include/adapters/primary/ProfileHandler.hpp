#pragma once

#include <IHttpHandler.hpp>
#include "ports/input/IAuthService.hpp"
#include "ports/output/IProfileRepository.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace adapters::primary {

/**
 * @brief Обработчик профиля пользователя
 * 
 * GET  /api/v1/profile - Получить свой профиль
 * PUT  /api/v1/profile - Обновить свой профиль
 * 
 * ВАЖНО: Пользователь видит и редактирует ТОЛЬКО свой профиль!
 * userId извлекается из JWT токена, а не из параметров запроса.
 */
class ProfileHandler : public IHttpHandler {
public:
    ProfileHandler(
        std::shared_ptr<ports::input::IAuthService> authService,
        std::shared_ptr<ports::output::IProfileRepository> profileRepository
    ) : authService_(std::move(authService))
      , profileRepository_(std::move(profileRepository))
    {
        std::cout << "[ProfileHandler] Created" << std::endl;
    }

    void handle(IRequest& req, IResponse& res) override {
        // Извлечение и валидация токена
        auto token = extractBearerToken(req);
        if (!token) {
            sendError(res, 401, "Authorization token required");
            return;
        }

        // Получение userId из токена
        auto userId = authService_->getUserIdFromToken(*token);
        if (!userId) {
            sendError(res, 401, "Invalid or expired token");
            return;
        }

        // Роутинг по методу
        std::string method = req.getMethod();

        if (method == "GET") {
            handleGet(res, *userId);
        } else if (method == "PUT") {
            handlePut(req, res, *userId);
        } else {
            sendError(res, 405, "Method not allowed");
        }
    }

private:
    std::shared_ptr<ports::input::IAuthService> authService_;
    std::shared_ptr<ports::output::IProfileRepository> profileRepository_;

    /**
     * @brief GET /api/v1/profile - Получить свой профиль
     */
    void handleGet(IResponse& res, const std::string& userId) {
        auto profile = profileRepository_->findByUserId(userId);

        if (!profile) {
            // Профиль должен был создаться при регистрации
            sendError(res, 404, "Profile not found");
            return;
        }

        sendJson(res, 200, profile->toJson());
    }

    /**
     * @brief PUT /api/v1/profile - Обновить свой профиль
     */
    void handlePut(IRequest& req, IResponse& res, const std::string& userId) {
        // Парсинг JSON
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.getBody());
        } catch (const std::exception& e) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        // Получение текущего профиля
        auto existingProfile = profileRepository_->findByUserId(userId);
        
        domain::Profile profile(userId);
        if (existingProfile) {
            profile = *existingProfile;
        }

        // Обновление полей (только переданные)
        if (body.contains("first_name") && body["first_name"].is_string()) {
            profile.firstName = body["first_name"].get<std::string>();
        }
        if (body.contains("last_name") && body["last_name"].is_string()) {
            profile.lastName = body["last_name"].get<std::string>();
        }
        if (body.contains("email") && body["email"].is_string()) {
            profile.email = body["email"].get<std::string>();
        }
        if (body.contains("phone") && body["phone"].is_string()) {
            profile.phone = body["phone"].get<std::string>();
        }

        // Сохранение
        try {
            profileRepository_->save(profile);
        } catch (const std::exception& e) {
            sendError(res, 500, "Failed to save profile");
            return;
        }

        // Возвращаем обновлённый профиль
        auto updatedProfile = profileRepository_->findByUserId(userId);
        if (updatedProfile) {
            sendJson(res, 200, updatedProfile->toJson());
        } else {
            sendJson(res, 200, profile.toJson());
        }
    }

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
