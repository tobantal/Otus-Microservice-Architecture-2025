#pragma once

#include <IHttpHandler.hpp>
#include <IRequest.hpp>
#include <IResponse.hpp>
#include <RouteMatcher.hpp>
#include "ports/output/IAccountRepository.hpp"
#include <nlohmann/json.hpp>
#include <memory>

namespace billing::adapters::primary {

class BillingHandler : public IHttpHandler {
public:
    explicit BillingHandler(std::shared_ptr<ports::output::IAccountRepository> repo) : repo_(repo) {}
    
    void handle(IRequest& req, IResponse& res) override {
        try {
            std::string method = req.getMethod();
            std::string path = req.getPath();
            
            if (method == "POST" && path == "/api/v1/billing/accounts") {
                auto body = nlohmann::json::parse(req.getBody());
                repo_->save({body["user_id"], 0});
                res.setStatus(201);
                res.setBody(R"({"status":"created"})");
            }
            else if (method == "POST" && path == "/api/v1/billing/deposit") {
                auto body = nlohmann::json::parse(req.getBody());
                std::string userId = body["user_id"];
                int64_t amount = body["amount"];
                auto acc = repo_->findByUserId(userId);
                if (acc) {
                    repo_->updateBalance(userId, acc->balance + amount);
                    res.setStatus(200);
                    res.setBody(nlohmann::json{{"balance", acc->balance + amount}}.dump());
                } else {
                    res.setStatus(404);
                    res.setBody(R"({"error":"Account not found"})");
                }
            }
            else if (method == "GET" && RouteMatcher::matches("/api/v1/billing/accounts/*", path)) {
                auto userId = path.substr(std::string("/api/v1/billing/accounts/").length());
                auto acc = repo_->findByUserId(userId);
                if (acc) {
                    res.setStatus(200);
                    res.setBody(nlohmann::json{{"user_id", acc->userId}, {"balance", acc->balance}}.dump());
                } else {
                    res.setStatus(404);
                    res.setBody(R"({"error":"Account not found"})");
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
    std::shared_ptr<ports::output::IAccountRepository> repo_;
};

} // namespace billing::adapters::primary
