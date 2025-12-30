#include "handlers/HealthCheckHandler.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

HealthCheckHandler::HealthCheckHandler() {
    std::cout << "[HealthCheckHandler] Created" << std::endl;
}

void HealthCheckHandler::handle(IRequest& req, IResponse& res) {
    json response;
    response["status"] = "OK";
    
    res.setStatus(200);
    res.setHeader("Content-Type", "application/json");
    res.setBody(response.dump());
}
