#pragma once

#include "ports/input/IBillingService.hpp"
#include "ports/output/IBillingRepository.hpp"
#include <memory>
#include <iostream>

namespace billing::application {

/**
 * @brief Реализация сервиса биллинга
 */
class BillingService : public ports::input::IBillingService {
public:
    explicit BillingService(std::shared_ptr<ports::output::IBillingRepository> repository)
        : repository_(std::move(repository))
    {
        std::cout << "[BillingService] Created" << std::endl;
    }

    ports::input::BillingResult createAccount(const std::string& userId) override {
        if (repository_->exists(userId)) {
            return {false, "Account already exists", 0};
        }

        domain::BillingAccount account(userId);
        repository_->save(account);

        std::cout << "[BillingService] Account created for user: " << userId << std::endl;
        return {true, "", 0};
    }

    ports::input::BillingResult deposit(const std::string& userId, int64_t amount) override {
        if (amount <= 0) {
            return {false, "Amount must be positive", 0};
        }

        auto account = repository_->findByUserId(userId);
        if (!account) {
            return {false, "Account not found", 0};
        }

        account->balance += amount;
        repository_->save(*account);

        std::cout << "[BillingService] Deposit " << amount 
                  << " for user: " << userId 
                  << ", new balance: " << account->balance << std::endl;
        
        return {true, "", account->balance};
    }

    ports::input::BillingResult charge(const std::string& userId, int64_t amount) override {
        if (amount <= 0) {
            return {false, "Amount must be positive", 0};
        }

        auto account = repository_->findByUserId(userId);
        if (!account) {
            return {false, "Account not found", 0};
        }

        if (account->balance < amount) {
            std::cout << "[BillingService] Insufficient funds for user: " << userId 
                      << ", balance: " << account->balance 
                      << ", requested: " << amount << std::endl;
            return {false, "Insufficient funds", account->balance};
        }

        account->balance -= amount;
        repository_->save(*account);

        std::cout << "[BillingService] Charged " << amount 
                  << " from user: " << userId 
                  << ", new balance: " << account->balance << std::endl;
        
        return {true, "", account->balance};
    }

    std::optional<domain::BillingAccount> getAccount(const std::string& userId) override {
        return repository_->findByUserId(userId);
    }

private:
    std::shared_ptr<ports::output::IBillingRepository> repository_;
};

} // namespace billing::application
