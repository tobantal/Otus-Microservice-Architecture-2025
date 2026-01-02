#pragma once

#include <string>
#include <cstdint>

namespace order::domain {

enum class SagaState {
    CREATED, BILLING_PENDING, BILLING_COMPLETED, WAREHOUSE_PENDING,
    WAREHOUSE_COMPLETED, DELIVERY_PENDING, COMPLETED,
    BILLING_FAILED, WAREHOUSE_FAILED, DELIVERY_FAILED, FAILED
};

inline std::string stateToString(SagaState s) {
    switch(s) {
        case SagaState::CREATED: return "CREATED";
        case SagaState::BILLING_PENDING: return "BILLING_PENDING";
        case SagaState::BILLING_COMPLETED: return "BILLING_COMPLETED";
        case SagaState::WAREHOUSE_PENDING: return "WAREHOUSE_PENDING";
        case SagaState::WAREHOUSE_COMPLETED: return "WAREHOUSE_COMPLETED";
        case SagaState::DELIVERY_PENDING: return "DELIVERY_PENDING";
        case SagaState::COMPLETED: return "COMPLETED";
        case SagaState::BILLING_FAILED: return "BILLING_FAILED";
        case SagaState::WAREHOUSE_FAILED: return "WAREHOUSE_FAILED";
        case SagaState::DELIVERY_FAILED: return "DELIVERY_FAILED";
        case SagaState::FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}

inline SagaState stringToState(const std::string& s) {
    if (s == "CREATED") return SagaState::CREATED;
    if (s == "BILLING_PENDING") return SagaState::BILLING_PENDING;
    if (s == "BILLING_COMPLETED") return SagaState::BILLING_COMPLETED;
    if (s == "WAREHOUSE_PENDING") return SagaState::WAREHOUSE_PENDING;
    if (s == "WAREHOUSE_COMPLETED") return SagaState::WAREHOUSE_COMPLETED;
    if (s == "DELIVERY_PENDING") return SagaState::DELIVERY_PENDING;
    if (s == "COMPLETED") return SagaState::COMPLETED;
    if (s == "BILLING_FAILED") return SagaState::BILLING_FAILED;
    if (s == "WAREHOUSE_FAILED") return SagaState::WAREHOUSE_FAILED;
    if (s == "DELIVERY_FAILED") return SagaState::DELIVERY_FAILED;
    if (s == "FAILED") return SagaState::FAILED;
    return SagaState::CREATED;
}

struct Order {
    std::string id;
    std::string userId;
    std::string productId;
    int64_t amount;
    int32_t quantity;
    SagaState state;
    std::string failureReason;
};

} // namespace order::domain
