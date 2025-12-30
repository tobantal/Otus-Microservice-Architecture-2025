#pragma once

#include <IHttpHandler.hpp>

/**
 * @brief Обработчик для проверки здоровья сервера
 * 
 * Endpoint: GET /health
 * Response: {"status": "OK"}
 */
class HealthCheckHandler : public IHttpHandler {
public:
    HealthCheckHandler();
    ~HealthCheckHandler() override = default;

    void handle(IRequest& req, IResponse& res) override;
};
