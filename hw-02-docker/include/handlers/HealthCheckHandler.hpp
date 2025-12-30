#pragma once

#include <IHttpHandler.hpp>
#include <string>
#include <vector>

/**
 * @class HealthCheckHandler
 * @brief Обработчик для проверки живости сервера
 * 
 * Endpoint: GET /health
 * 
 * Response (200 OK):
 * {
 *   "status": "ok"
 *   }
 * }
 */
class HealthCheckHandler : public IHttpHandler
{
public:
    HealthCheckHandler();
    virtual ~HealthCheckHandler() = default;

    /**
     * @brief Обработать HTTP запрос проверки здоровья сервера
     */
    void handle(IRequest& req, IResponse& res) override;

private:
    /**
     * @brief Построить JSON ответ о состоянии сервера
     */
    std::string buildHealthJson() const;
};