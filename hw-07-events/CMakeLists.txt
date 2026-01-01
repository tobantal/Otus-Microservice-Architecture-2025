cmake_minimum_required(VERSION 3.15)

project(hw07-events VERSION 1.0.0 LANGUAGES CXX)

# Подключаем все микросервисы
add_subdirectory(billing-service)
add_subdirectory(notification-service)
add_subdirectory(order-service)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  HW07 Event-Driven Architecture")
message(STATUS "  Services: billing, notification, order")
message(STATUS "========================================")
message(STATUS "")
