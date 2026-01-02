#include "BillingApp.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Saga Billing Service ===" << std::endl;
    try {
        billing::BillingApp app;
        app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
