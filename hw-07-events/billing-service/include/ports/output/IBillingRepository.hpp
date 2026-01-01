#pragma once

#include "domain/BillingAccount.hpp"
#include <optional>

namespace billing::ports::output {

/**
 * @brief Репозиторий биллинговых аккаунтов
 */
class IBillingRepository {
public:
    virtual ~IBillingRepository() = default;

    virtual void save(const domain::BillingAccount& account) = 0;
    virtual std::optional<domain::BillingAccount> findByUserId(const std::string& userId) = 0;
    virtual bool exists(const std::string& userId) = 0;
};

} // namespace billing::ports::output
