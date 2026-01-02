#pragma once

#include <BoostBeastApplication.hpp>

namespace warehouse {

class WarehouseApp : public BoostBeastApplication {
public:
    WarehouseApp();
    ~WarehouseApp();
protected:
    void configureInjection() override;
};

} // namespace warehouse
