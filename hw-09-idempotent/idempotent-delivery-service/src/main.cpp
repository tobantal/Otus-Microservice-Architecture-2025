#include "DeliveryApp.hpp"
#include <iostream>
#include <csignal>

delivery::DeliveryApp* g_app = nullptr;

void signalHandler(int signal) {
    std::cout << "\n[main] Received signal " << signal << std::endl;
    if (g_app) {
        g_app->stop();
    }
}

int main(int argc, char* argv[]) {
    try {
        delivery::DeliveryApp app;
        g_app = &app;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        std::cout << "========================================" << std::endl;
        std::cout << "  HW09 Idempotent Delivery Service" << std::endl;
        std::cout << "  Press Ctrl+C to stop" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        app.run(argc, argv);

        std::cout << "\n========================================" << std::endl;
        std::cout << "  Delivery Service Stopped" << std::endl;
        std::cout << "========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[main] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}