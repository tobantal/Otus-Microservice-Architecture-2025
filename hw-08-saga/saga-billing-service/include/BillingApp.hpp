#pragma once

#include <BoostBeastApplication.hpp>

namespace billing {

class BillingApp : public BoostBeastApplication {
public:
    BillingApp();
    ~BillingApp();
protected:
    void configureInjection() override;
};

} // namespace billing
