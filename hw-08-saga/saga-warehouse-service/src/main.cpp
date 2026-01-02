#include "WarehouseApp.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Saga Warehouse Service ===" << std::endl;
    try {
        warehouse::WarehouseApp app;
        app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
