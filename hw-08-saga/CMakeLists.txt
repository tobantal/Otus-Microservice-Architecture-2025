cmake_minimum_required(VERSION 3.15)
project(hw08-saga VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(saga-order-service)
add_subdirectory(saga-billing-service)
add_subdirectory(saga-warehouse-service)
add_subdirectory(saga-delivery-service)

message(STATUS "")
message(STATUS "=== HW08: Choreography Saga ===")
message(STATUS "Services: saga-order, saga-billing, saga-warehouse, saga-delivery")
message(STATUS "")
