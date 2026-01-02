#include "DeliveryApp.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Saga Delivery Service ===" << std::endl;
    try {
        delivery::DeliveryApp app;
        app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
