#pragma once

#include <IHttpHandler.hpp>
#include "ports/input/IBillingService.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>
#include <regex>

namespace billing::adapters::primary {

/**
 * @brief HTTP Handler для биллинга
 * 
 * Endpoints:
 * - POST /api/v1/billing/accounts      - создать аккаунт
 * - POST /api/v1/billing/deposit       - пополнить
 * - POST /api/v1/billing/charge        - списать
 * - GET  /api/v1/billing/accounts/{id} - получить баланс
 */
class BillingHandler : public IHttpHandler {
public:
    explicit BillingHandler(std::shared_ptr<ports::input::IBillingService> billingService)
        : billingService_(std::move(billingService))
    {
        std::cout << "[BillingHandler] Created" << std::endl;
    }

    void handle(IRequest& req, IResponse& res) override {
        std::string method = req.getMethod();
        std::string path = req.getPath();

        if (method == "POST" && path == "/api/v1/billing/accounts") {
            handleCreateAccount(req, res);
        } else if (method == "POST" && path == "/api/v1/billing/deposit") {
            handleDeposit(req, res);
        } else if (method == "POST" && path == "/api/v1/billing/charge") {
            handleCharge(req, res);
        } else if (method == "GET" && path.find("/api/v1/billing/accounts/") == 0) {
            handleGetAccount(req, res);
        } else {
            sendError(res, 404, "Not found");
        }
    }

private:
    std::shared_ptr<ports::input::IBillingService> billingService_;

    /**
     * @brief POST /api/v1/billing/accounts
     * Body: {"user_id": "..."}
     */
    void handleCreateAccount(IRequest& req, IResponse& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.getBody());
        } catch (...) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        std::string userId = body.value("user_id", "");
        if (userId.empty()) {
            sendError(res, 400, "user_id is required");
            return;
        }

        auto result = billingService_->createAccount(userId);
        if (!result.success) {
            sendError(res, 409, result.error);
            return;
        }

        nlohmann::json response = {
            {"user_id", userId},
            {"balance", 0},
            {"message", "Account created"}
        };
        sendJson(res, 201, response);
    }

    /**
     * @brief POST /api/v1/billing/deposit
     * Body: {"user_id": "...", "amount": 1000}
     */
    void handleDeposit(IRequest& req, IResponse& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.getBody());
        } catch (...) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        std::string userId = body.value("user_id", "");
        int64_t amount = body.value("amount", int64_t{0});

        if (userId.empty()) {
            sendError(res, 400, "user_id is required");
            return;
        }
        if (amount <= 0) {
            sendError(res, 400, "amount must be positive");
            return;
        }

        auto result = billingService_->deposit(userId, amount);
        if (!result.success) {
            sendError(res, 400, result.error);
            return;
        }

        nlohmann::json response = {
            {"user_id", userId},
            {"balance", result.newBalance},
            {"message", "Deposit successful"}
        };
        sendJson(res, 200, response);
    }

    /**
     * @brief POST /api/v1/billing/charge
     * Body: {"user_id": "...", "amount": 500}
     */
    void handleCharge(IRequest& req, IResponse& res) {
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.getBody());
        } catch (...) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        std::string userId = body.value("user_id", "");
        int64_t amount = body.value("amount", int64_t{0});

        if (userId.empty()) {
            sendError(res, 400, "user_id is required");
            return;
        }
        if (amount <= 0) {
            sendError(res, 400, "amount must be positive");
            return;
        }

        auto result = billingService_->charge(userId, amount);
        if (!result.success) {
            // Insufficient funds или account not found
            int status = (result.error == "Insufficient funds") ? 402 : 404;
            sendError(res, status, result.error);
            return;
        }

        nlohmann::json response = {
            {"user_id", userId},
            {"balance", result.newBalance},
            {"message", "Charge successful"}
        };
        sendJson(res, 200, response);
    }

    /**
     * @brief GET /api/v1/billing/accounts/{userId}
     */
    void handleGetAccount(IRequest& req, IResponse& res) {
        std::string path = req.getPath();
        
        // Извлекаем userId из пути
        std::regex pathRegex(R"(/api/v1/billing/accounts/([^/]+))");
        std::smatch match;
        
        if (!std::regex_match(path, match, pathRegex)) {
            sendError(res, 400, "Invalid path");
            return;
        }

        std::string userId = match[1].str();
        auto account = billingService_->getAccount(userId);

        if (!account) {
            sendError(res, 404, "Account not found");
            return;
        }

        sendJson(res, 200, account->toJson());
    }

    void sendJson(IResponse& res, int status, const nlohmann::json& body) {
        res.setStatus(status);
        res.setHeader("Content-Type", "application/json");
        res.setBody(body.dump());
    }

    void sendError(IResponse& res, int status, const std::string& message) {
        nlohmann::json body = {{"error", message}};
        sendJson(res, status, body);
    }
};

} // namespace billing::adapters::primary
