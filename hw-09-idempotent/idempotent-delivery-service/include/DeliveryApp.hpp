#pragma once

#include <BoostBeastApplication.hpp>

namespace delivery {

class DeliveryApp : public BoostBeastApplication {
public:
    DeliveryApp();
    ~DeliveryApp();
protected:
    void configureInjection() override;
};

} // namespace delivery
