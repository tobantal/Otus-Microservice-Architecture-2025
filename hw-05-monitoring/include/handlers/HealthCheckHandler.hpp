#pragma once

#include <IHttpHandler.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

/**
 * @brief Обработчик для проверки живости сервера
 * 
 * Endpoint: GET /health
 */
class HealthCheckHandler : public IHttpHandler {
public:
    HealthCheckHandler() {
        std::cout << "[HealthCheckHandler] Created" << std::endl;
    }
    
    virtual ~HealthCheckHandler() = default;

    void handle(IRequest& req, IResponse& res) override {
        std::cout << "[HealthCheckHandler] Handling request from " << req.getIp() << std::endl;
        
        nlohmann::json response;
        response["status"] = "ok";
        
        res.setStatus(200);
        res.setHeader("Content-Type", "application/json");
        res.setBody(response.dump());
    }
};
