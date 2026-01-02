#pragma once

#include <IHttpHandler.hpp>
#include <IRequest.hpp>
#include <IResponse.hpp>
#include <RouteMatcher.hpp>
#include "ports/output/IProductRepository.hpp"
#include <nlohmann/json.hpp>
#include <memory>

namespace warehouse::adapters::primary {

class WarehouseHandler : public IHttpHandler {
public:
    explicit WarehouseHandler(std::shared_ptr<ports::output::IProductRepository> repo) : repo_(repo) {}
    
    void handle(IRequest& req, IResponse& res) override {
        try {
            if (req.getMethod() == "POST" && req.getPath() == "/api/v1/warehouse/products") {
                auto body = nlohmann::json::parse(req.getBody());
                repo_->save({body["product_id"], body["quantity"], 0});
                res.setStatus(201);
                res.setBody(R"({"status":"created"})");
            }
            else if (req.getMethod() == "GET" && RouteMatcher::matches("/api/v1/warehouse/stock/*", req.getPath())) {
                auto id = req.getPath().substr(std::string("/api/v1/warehouse/stock/").length());
                auto p = repo_->findById(id);
                if (p) {
                    res.setStatus(200);
                    res.setBody(nlohmann::json{
                        {"product_id", p->productId}, {"stock", p->stock},
                        {"reserved", p->reserved}, {"available", p->stock - p->reserved}
                    }.dump());
                } else {
                    res.setStatus(404);
                    res.setBody(R"({"error":"Not found"})");
                }
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
    std::shared_ptr<ports::output::IProductRepository> repo_;
};

} // namespace warehouse::adapters::primary
