cmake_minimum_required(VERSION 3.15)
project(hw09-idempotent VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(idempotent-order-service)
add_subdirectory(idempotent-billing-service)
add_subdirectory(idempotent-warehouse-service)
add_subdirectory(idempotent-delivery-service)

message(STATUS "")
message(STATUS "=== HW09: Idempotent pattern ===")
message(STATUS "Services: idempotent-order, idempotent-billing, idempotent-warehouse, idempotent-delivery")
message(STATUS "")
