cmake_minimum_required(VERSION 3.14)
project(otus-microservices VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

# cpp-http-server (включает Boost.Beast, Boost.DI, nlohmann/json)
FetchContent_Declare(
    cpp-http-server
    GIT_REPOSITORY https://github.com/tobantal/cpp-http-server.git
    GIT_TAG        v0.0.5
)


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

message(STATUS "")
message(STATUS "Configuration completed successfully")
message(STATUS "")
