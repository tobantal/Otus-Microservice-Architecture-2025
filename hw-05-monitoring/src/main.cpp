#include "MonitoringApp.hpp"
#include <iostream>
#include <csignal>

// Глобальный указатель для обработки сигналов
MonitoringApp* g_app = nullptr;

void signalHandler(int signal) {
    std::cout << "\n[main] Received signal " << signal << std::endl;
    if (g_app) {
        g_app->stop();
    }
}

int main(int argc, char* argv[]) {
    try {
        MonitoringApp app;
        g_app = &app;

        // Устанавливаем обработчики сигналов для graceful shutdown
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        std::cout << "========================================" << std::endl;
        std::cout << "  HW05 Monitoring Service" << std::endl;
        std::cout << "  Prometheus metrics enabled" << std::endl;
        std::cout << "  Press Ctrl+C to stop" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        app.run(argc, argv);

        std::cout << "\n========================================" << std::endl;
        std::cout << "  Monitoring Service Stopped" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[main] Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
