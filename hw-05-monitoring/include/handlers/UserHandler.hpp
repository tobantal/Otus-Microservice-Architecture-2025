#pragma once

#include <IHttpHandler.hpp>
#include "repository/IUserRepository.hpp"
#include "domain/User.hpp"
#include "metrics/PrometheusMetrics.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>
#include <regex>

using json = nlohmann::json;

/**
 * @brief HTTP Handler для CRUD операций с пользователями
 * 
 * С интегрированными Prometheus метриками:
 * - Latency (histogram с квантилями)
 * - Request count по методам/статусам
 * - Error count (5xx)
 * 
 * Endpoints:
 * - POST   /api/v1/users      - создать пользователя
 * - GET    /api/v1/users      - получить всех пользователей
 * - GET    /api/v1/users/{id} - получить пользователя по ID
 * - PUT    /api/v1/users/{id} - обновить пользователя
 * - DELETE /api/v1/users/{id} - удалить пользователя
 */
class UserHandler : public IHttpHandler {
public:
    explicit UserHandler(std::shared_ptr<repository::IUserRepository> userRepo)
        : userRepository_(std::move(userRepo))
    {
        std::cout << "[UserHandler] Created with repository" << std::endl;
    }
    
    virtual ~UserHandler() = default;
    
    void handle(IRequest& req, IResponse& res) override {
        std::string method = req.getMethod();
        std::string path = req.getPath();
        
        // Запускаем таймер для измерения latency
        metrics::ScopedTimer timer(method, path);
        
        std::cout << "[UserHandler] " << method << " " << path << std::endl;
        
        int statusCode = 200;
        
        try {
            if (method == "POST" && path == "/api/v1/users") {
                statusCode = handleCreate(req, res);
            } else if (method == "GET" && path == "/api/v1/users") {
                statusCode = handleGetAll(req, res);
            } else if (method == "GET") {
                statusCode = handleGetById(req, res);
            } else if (method == "PUT") {
                statusCode = handleUpdate(req, res);
            } else if (method == "DELETE") {
                statusCode = handleDelete(req, res);
            } else {
                statusCode = 405;
                res.setStatus(405);
                res.setHeader("Content-Type", "application/json");
                res.setBody(R"({"error": "Method not allowed"})");
            }
        } catch (const std::exception& e) {
            std::cerr << "[UserHandler] Exception: " << e.what() << std::endl;
            statusCode = 500;
            res.setStatus(500);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "Internal server error"})");
        }
        
        // Записываем метрики
        metrics::MetricsRegistry::instance().incrementRequestsTotal(method, path, statusCode);
        if (statusCode >= 500) {
            metrics::MetricsRegistry::instance().incrementErrors(method, path);
        }
    }

private:
    int handleCreate(IRequest& req, IResponse& res) {
        try {
            auto body = json::parse(req.getBody());
            domain::User user = domain::User::fromJson(body);
            
            int64_t id = userRepository_->create(user);
            user.id = id;
            
            res.setStatus(201);
            res.setHeader("Content-Type", "application/json");
            res.setBody(user.toJson().dump());
            return 201;
        } catch (const json::exception& e) {
            res.setStatus(400);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "Invalid JSON"})");
            return 400;
        }
    }
    
    int handleGetAll(IRequest& req, IResponse& res) {
        auto users = userRepository_->findAll();
        
        json arr = json::array();
        for (const auto& user : users) {
            arr.push_back(user.toJson());
        }
        
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(arr.dump());
        return 200;
    }
    
    int handleGetById(IRequest& req, IResponse& res) {
        auto id = extractIdFromPath(req.getPath());
        if (!id) {
            res.setStatus(400);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "Invalid user ID"})");
            return 400;
        }
        
        auto user = userRepository_->findById(*id);
        if (!user) {
            res.setStatus(404);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "User not found"})");
            return 404;
        }
        
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(user->toJson().dump());
        return 200;
    }
    
    int handleUpdate(IRequest& req, IResponse& res) {
        auto id = extractIdFromPath(req.getPath());
        if (!id) {
            res.setStatus(400);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "Invalid user ID"})");
            return 400;
        }
        
        try {
            auto body = json::parse(req.getBody());
            domain::User user = domain::User::fromJson(body);
            
            bool updated = userRepository_->update(*id, user);
            if (!updated) {
                res.setStatus(404);
                res.setHeader("Content-Type", "application/json");
                res.setBody(R"({"error": "User not found"})");
                return 404;
            }
            
            user.id = *id;
            res.setStatus(200);
            res.setHeader("Content-Type", "application/json");
            res.setBody(user.toJson().dump());
            return 200;
        } catch (const json::exception& e) {
            res.setStatus(400);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "Invalid JSON"})");
            return 400;
        }
    }
    
    int handleDelete(IRequest& req, IResponse& res) {
        auto id = extractIdFromPath(req.getPath());
        if (!id) {
            res.setStatus(400);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "Invalid user ID"})");
            return 400;
        }
        
        bool deleted = userRepository_->deleteById(*id);
        if (!deleted) {
            res.setStatus(404);
            res.setHeader("Content-Type", "application/json");
            res.setBody(R"({"error": "User not found"})");
            return 404;
        }
        
        res.setStatus(204);
        return 204;
    }
    
    std::optional<int64_t> extractIdFromPath(const std::string& path) {
        // /api/v1/users/123 -> 123
        std::regex re(R"(/api/v1/users/(\d+))");
        std::smatch match;
        
        if (std::regex_match(path, match, re)) {
            try {
                return std::stoll(match[1].str());
            } catch (...) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
    
    std::shared_ptr<repository::IUserRepository> userRepository_;
};
