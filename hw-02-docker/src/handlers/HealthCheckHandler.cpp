#include "handlers/HealthCheckHandler.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

HealthCheckHandler::HealthCheckHandler()
{
    std::cout << "[HealthCheckHandler] Created" << std::endl;
}

void HealthCheckHandler::handle(IRequest& req, IResponse& res)
{
    std::cout << "[HealthCheckHandler] Handling request from " << req.getIp() << std::endl;
    
    // Устанавливаем статус ответа
    res.setStatus(200);
    res.setHeader("Content-Type", "application/json");
    
    // Устанавливаем тело ответа
    res.setBody(buildHealthJson());
    
    std::cout << "[HealthCheckHandler] Response sent" << std::endl;
}

std::string HealthCheckHandler::buildHealthJson() const
{
    json response;
    response["status"] = "ok";
    return response.dump();
}