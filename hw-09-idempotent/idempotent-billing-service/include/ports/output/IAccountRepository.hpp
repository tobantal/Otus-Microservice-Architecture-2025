#pragma once

#include "domain/Account.hpp"
#include <string>
#include <optional>

namespace billing::ports::output {

class IAccountRepository {
public:
    virtual ~IAccountRepository() = default;
    virtual void save(const domain::Account& acc) = 0;
    virtual void updateBalance(const std::string& userId, int64_t balance) = 0;
    virtual std::optional<domain::Account> findByUserId(const std::string& userId) = 0;
};

} // namespace billing::ports::output
