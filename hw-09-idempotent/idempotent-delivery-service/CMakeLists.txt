cmake_minimum_required(VERSION 3.15)
project(idempotent-delivery-service VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(NOT TARGET microservice-core)
    FetchContent_Declare(cpp-http-server
        GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
        GIT_TAG v0.0.5)
    FetchContent_MakeAvailable(cpp-http-server)
endif()

find_package(PostgreSQL REQUIRED)

if(NOT TARGET pqxx)
    FetchContent_Declare(libpqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG 7.9.2)
    set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
    set(BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(BUILD_TEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(libpqxx)
endif()

find_package(OpenSSL REQUIRED)

if(NOT TARGET amqpcpp)
    FetchContent_Declare(amqpcpp
        GIT_REPOSITORY https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
        GIT_TAG v4.3.26)
    set(AMQP-CPP_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(AMQP-CPP_LINUX_TCP ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(amqpcpp)
endif()

find_package(Threads REQUIRED)

add_executable(idempotent-delivery-service
    src/main.cpp
    src/DeliveryApp.cpp
    src/RabbitMQAdapter.cpp
)

target_include_directories(idempotent-delivery-service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(idempotent-delivery-service PRIVATE
    microservice-core
    microservice-boost
    pqxx
    amqpcpp
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
