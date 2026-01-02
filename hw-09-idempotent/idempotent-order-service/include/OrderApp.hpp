#pragma once

#include <BoostBeastApplication.hpp>

namespace order {

class OrderApp : public BoostBeastApplication {
public:
    OrderApp();
    ~OrderApp();
protected:
    void configureInjection() override;
};

} // namespace order
