#include "WarehouseApp.hpp"
#include <iostream>
#include <csignal>

warehouse::WarehouseApp* g_app = nullptr;

void signalHandler(int signal) {
    std::cout << "\n[main] Received signal " << signal << std::endl;
    if (g_app) {
        g_app->stop();
    }
}

int main(int argc, char* argv[]) {
    try {
        warehouse::WarehouseApp app;
        g_app = &app;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        std::cout << "========================================" << std::endl;
        std::cout << "  HW09 Idempotent Warehouse Service" << std::endl;
        std::cout << "  Press Ctrl+C to stop" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        app.run(argc, argv);

        std::cout << "\n========================================" << std::endl;
        std::cout << "  Warehouse Service Stopped" << std::endl;
        std::cout << "========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[main] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}