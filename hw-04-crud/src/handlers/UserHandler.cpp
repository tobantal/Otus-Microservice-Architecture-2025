#include "handlers/UserHandler.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <regex>

using json = nlohmann::json;

UserHandler::UserHandler(std::shared_ptr<repository::IUserRepository> repository)
    : repository_(std::move(repository))
{
    std::cout << "[UserHandler] Created" << std::endl;
}

void UserHandler::handle(IRequest& req, IResponse& res) {
    const std::string& method = req.getMethod();
    const std::string& path = req.getPath();
    
    std::cout << "[UserHandler] " << method << " " << path << std::endl;
    
    // Извлекаем ID из пути (если есть)
    int64_t id = extractIdFromPath(path);
    
    try {
        if (method == "POST" && id == -1) {
            // POST /api/v1/users
            handleCreate(req, res);
        }
        else if (method == "GET" && id == -1) {
            // GET /api/v1/users
            handleGetAll(req, res);
        }
        else if (method == "GET" && id != -1) {
            // GET /api/v1/users/{id}
            handleGetById(req, res, id);
        }
        else if (method == "PUT" && id != -1) {
            // PUT /api/v1/users/{id}
            handleUpdate(req, res, id);
        }
        else if (method == "DELETE" && id != -1) {
            // DELETE /api/v1/users/{id}
            handleDelete(req, res, id);
        }
        else {
            sendError(res, 405, "Method Not Allowed");
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[UserHandler] Error: " << e.what() << std::endl;
        sendError(res, 500, e.what());
    }
}

void UserHandler::handleCreate(IRequest& req, IResponse& res) {
    // Парсим тело запроса
    std::string body = req.getBody();
    if (body.empty()) {
        sendError(res, 400, "Request body is empty");
        return;
    }
    
    json requestJson;
    try {
        requestJson = json::parse(body);
    } catch (const json::exception& e) {
        sendError(res, 400, "Invalid JSON: " + std::string(e.what()));
        return;
    }
    
    // Создаём пользователя из JSON
    domain::User user = domain::User::fromJson(requestJson);
    
    // Валидация
    if (!user.isValid()) {
        sendError(res, 400, "Missing required fields: username and email are required");
        return;
    }
    
    // Сохраняем в БД
    int64_t id = repository_->create(user);
    user.id = id;
    
    // Возвращаем созданного пользователя
    sendJson(res, 201, user.toJson());
}

void UserHandler::handleGetAll(IRequest& req, IResponse& res) {
    auto users = repository_->findAll();
    
    json response = json::array();
    for (const auto& user : users) {
        response.push_back(user.toJson());
    }
    
    sendJson(res, 200, response);
}

void UserHandler::handleGetById(IRequest& req, IResponse& res, int64_t id) {
    auto user = repository_->findById(id);
    
    if (!user) {
        sendError(res, 404, "User not found");
        return;
    }
    
    sendJson(res, 200, user->toJson());
}

void UserHandler::handleUpdate(IRequest& req, IResponse& res, int64_t id) {
    // Проверяем существование
    auto existing = repository_->findById(id);
    if (!existing) {
        sendError(res, 404, "User not found");
        return;
    }
    
    // Парсим тело запроса
    std::string body = req.getBody();
    if (body.empty()) {
        sendError(res, 400, "Request body is empty");
        return;
    }
    
    json requestJson;
    try {
        requestJson = json::parse(body);
    } catch (const json::exception& e) {
        sendError(res, 400, "Invalid JSON: " + std::string(e.what()));
        return;
    }
    
    // Обновляем поля
    domain::User user = domain::User::fromJson(requestJson);
    user.id = id;
    
    // Валидация
    if (!user.isValid()) {
        sendError(res, 400, "Missing required fields: username and email are required");
        return;
    }
    
    // Сохраняем
    repository_->update(id, user);
    
    sendJson(res, 200, user.toJson());
}

void UserHandler::handleDelete(IRequest& req, IResponse& res, int64_t id) {
    bool deleted = repository_->deleteById(id);
    
    if (!deleted) {
        sendError(res, 404, "User not found");
        return;
    }
    
    // 204 No Content
    res.setStatus(204);
    res.setBody("");
}

int64_t UserHandler::extractIdFromPath(const std::string& path) {
    // Паттерн: /api/v1/users/{id}
    std::regex idRegex(R"(/api/v1/users/(\d+))");
    std::smatch match;
    
    if (std::regex_match(path, match, idRegex)) {
        return std::stoll(match[1].str());
    }
    
    return -1;  // ID не найден
}

void UserHandler::sendJson(IResponse& res, int status, const nlohmann::json& body) {
    res.setStatus(status);
    res.setHeader("Content-Type", "application/json");
    res.setBody(body.dump());
}

void UserHandler::sendError(IResponse& res, int status, const std::string& message) {
    json error;
    error["code"] = status;
    error["message"] = message;
    
    res.setStatus(status);
    res.setHeader("Content-Type", "application/json");
    res.setBody(error.dump());
}
