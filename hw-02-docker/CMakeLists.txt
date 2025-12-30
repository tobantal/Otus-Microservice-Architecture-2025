cmake_minimum_required(VERSION 3.15)

project(helloworld
    VERSION 1.0.0
    LANGUAGES CXX
)

include(FetchContent)

# ============================================================================
# Dependencies
# ============================================================================

FetchContent_Declare(
    cpp_http_server
    GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
    GIT_TAG v0.0.5
)

FetchContent_MakeAvailable(
    cpp_http_server
)

find_package(Threads REQUIRED)

# ============================================================================
# Application
# ============================================================================

add_executable(helloworld
    src/main.cpp
    src/HelloWorldApp.cpp
    src/handlers/HealthCheckHandler.cpp
)

target_include_directories(helloworld
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ============================================================================
# Link libraries (ТОЛЬКО targets!)
# ============================================================================

target_link_libraries(helloworld
    PRIVATE
        microservice-core
        microservice-boost
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
message(STATUS "  HelloWorld Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "========================================")
message(STATUS "")
