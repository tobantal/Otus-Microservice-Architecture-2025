cmake_minimum_required(VERSION 3.15)
project(otus-microservices VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# ============================================
# ЗАВИСИМОСТИ: FetchContent
# ============================================
include(FetchContent)

# cpp-http-server (включает Boost.Beast, Boost.DI, nlohmann/json)
FetchContent_Declare(
    cpp-http-server
    GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
    GIT_TAG        v0.0.5
)

FetchContent_MakeAvailable(cpp-http-server)

# Найти OpenSSL (ДО FetchContent amqpcpp)
find_package(OpenSSL REQUIRED)

# AMQP-CPP для RabbitMQ
FetchContent_Declare(
    amqpcpp
    GIT_REPOSITORY https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
    GIT_TAG v4.3.26
)
# КРИТИЧНО: включить TCP поддержку
set(AMQP-CPP_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(AMQP-CPP_LINUX_TCP ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(amqpcpp)

# Сначала получаем libpq (PostgreSQL C library)
find_package(PostgreSQL REQUIRED)

# Получаем libpqxx
FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
)

# Опции для libpqxx (отключаем ненужное)
set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
set(BUILD_DOC OFF CACHE BOOL "" FORCE)
set(BUILD_TEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libpqxx)

# System dependencies
find_package(Threads REQUIRED)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  OTUS Microservices Homework")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "========================================")
message(STATUS "")

# Common libraries
#add_subdirectory(common)

# Homework projects
add_subdirectory(hw-02-docker)

add_subdirectory(hw-04-crud)

add_subdirectory(hw-05-monitoring)

add_subdirectory(hw-06-auth)

add_subdirectory(hw-07-events)

add_subdirectory(hw-08-saga)

message(STATUS "")
message(STATUS "Configuration completed successfully")
message(STATUS "")
