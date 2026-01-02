#pragma once

#include <IHttpHandler.hpp>
#include <IRequest.hpp>
#include <IResponse.hpp>
#include "ports/output/ISlotRepository.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <chrono>

namespace delivery::adapters::primary {

class DeliveryHandler : public IHttpHandler {
public:
    explicit DeliveryHandler(std::shared_ptr<ports::output::ISlotRepository> repo) : repo_(repo) {}
    
    void handle(IRequest& req, IResponse& res) override {
        try {
            if (req.getMethod() == "POST" && req.getPath() == "/api/v1/delivery/slots") {
                auto body = nlohmann::json::parse(req.getBody());
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                static int c = 0;
                std::string id = "slot-" + std::to_string(ms) + "-" + std::to_string(++c);
                repo_->save({id, body["slot_time"], "", "available"});
                res.setStatus(201);
                res.setBody(nlohmann::json{{"slot_id", id}}.dump());
            }
            else if (req.getMethod() == "DELETE" && req.getPath() == "/api/v1/delivery/slots") {
                repo_->deleteAll();
                res.setStatus(200);
                res.setBody(R"({"status":"deleted"})");
            }
            else {
                res.setStatus(404);
                res.setBody(R"({"error":"Not found"})");
            }
            res.setHeader("Content-Type", "application/json");
        } catch (const std::exception& e) {
            res.setStatus(500);
            res.setHeader("Content-Type", "application/json");
            res.setBody(nlohmann::json{{"error", e.what()}}.dump());
        }
    }

private:
    std::shared_ptr<ports::output::ISlotRepository> repo_;
};

} // namespace delivery::adapters::primary
