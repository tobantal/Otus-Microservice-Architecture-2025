cmake_minimum_required(VERSION 3.15)

project(notification-service
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ============================================================================
# Dependencies
# ============================================================================

# cpp-http-server (Boost.Beast, Boost.DI, nlohmann/json)
if(NOT TARGET microservice-core)
    FetchContent_Declare(
        cpp-http-server
        GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
        GIT_TAG        v0.0.5
    )
    FetchContent_MakeAvailable(cpp-http-server)
endif()

# PostgreSQL C library
find_package(PostgreSQL REQUIRED)

# libpqxx (C++ wrapper для PostgreSQL)
if(NOT TARGET pqxx)
    FetchContent_Declare(
        libpqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG 7.9.2
    )
    set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
    set(BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(BUILD_TEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(libpqxx)
endif()

find_package(Threads REQUIRED)

# ============================================================================
# Application
# ============================================================================

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/NotificationApp.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        microservice-core
        microservice-boost
        pqxx
        Threads::Threads
)

# ============================================================================
# Copy config.json to build directory
# ============================================================================

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
    COPYONLY
)

# ============================================================================
# Diagnostics
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Notification Service Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "========================================")
