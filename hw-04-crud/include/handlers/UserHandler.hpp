#pragma once

#include <IHttpHandler.hpp>
#include "repository/IUserRepository.hpp"
#include <memory>

/**
 * @brief Обработчик CRUD операций над пользователями
 * 
 * Endpoints:
 * - POST   /api/v1/users     - Создать пользователя
 * - GET    /api/v1/users     - Список всех пользователей
 * - GET    /api/v1/users/{id} - Получить по ID
 * - PUT    /api/v1/users/{id} - Обновить
 * - DELETE /api/v1/users/{id} - Удалить
 */
class UserHandler : public IHttpHandler {
public:
    explicit UserHandler(std::shared_ptr<repository::IUserRepository> repository);
    ~UserHandler() override = default;

    void handle(IRequest& req, IResponse& res) override;

private:
    /**
     * @brief POST /api/v1/users - Создать пользователя
     */
    void handleCreate(IRequest& req, IResponse& res);

    /**
     * @brief GET /api/v1/users - Список всех
     */
    void handleGetAll(IRequest& req, IResponse& res);

    /**
     * @brief GET /api/v1/users/{id} - Получить по ID
     */
    void handleGetById(IRequest& req, IResponse& res, int64_t id);

    /**
     * @brief PUT /api/v1/users/{id} - Обновить
     */
    void handleUpdate(IRequest& req, IResponse& res, int64_t id);

    /**
     * @brief DELETE /api/v1/users/{id} - Удалить
     */
    void handleDelete(IRequest& req, IResponse& res, int64_t id);

    /**
     * @brief Извлечь ID из пути /api/v1/users/{id}
     * @return ID или -1 если не найден
     */
    int64_t extractIdFromPath(const std::string& path);

    /**
     * @brief Отправить JSON ответ
     */
    void sendJson(IResponse& res, int status, const nlohmann::json& body);

    /**
     * @brief Отправить ошибку
     */
    void sendError(IResponse& res, int status, const std::string& message);

    std::shared_ptr<repository::IUserRepository> repository_;
};
