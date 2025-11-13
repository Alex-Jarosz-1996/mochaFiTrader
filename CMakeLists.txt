cmake_minimum_required(VERSION 3.15)
project(mochaFiTrader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# -------------------------
# Source files
# -------------------------
set(SOURCES
    src/main.cpp
    src/tastyworks/TastyWorks.cpp
    src/streamer/DX_LinkStreamer.cpp
    src/database/DB_Client.cpp
    src/config/Config.cpp
    src/log/Log.cpp
)

# Your project headers
include_directories(
    ${CMAKE_SOURCE_DIR}/src/tastyworks
    ${CMAKE_SOURCE_DIR}/src/streamer
    ${CMAKE_SOURCE_DIR}/src/database
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/log
)

# -------------------------
# Dependencies
# -------------------------
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

# PostgreSQL C library (libpq)
find_package(PostgreSQL REQUIRED)

# logging
find_package(spdlog CONFIG REQUIRED)

# libpqxx (via pkg-config since no CMake config file exists)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

# -------------------------
# Executable
# -------------------------
add_executable(mochaFiTrader ${SOURCES})

target_include_directories(mochaFiTrader PRIVATE
    ${LIBPQXX_INCLUDE_DIRS}
)

target_link_directories(mochaFiTrader PRIVATE
    ${LIBPQXX_LIBRARY_DIRS}
)

target_link_libraries(mochaFiTrader PRIVATE 
    cpr::cpr 
    nlohmann_json::nlohmann_json
    websocketpp::websocketpp
    Boost::boost
    Threads::Threads
    ${LIBPQXX_LIBRARIES}
    PostgreSQL::PostgreSQL
    spdlog::spdlog
)

# -------------------------
# Unit Tests
# -------------------------
enable_testing()
find_package(GTest REQUIRED)
find_package(httplib CONFIG REQUIRED)

add_executable(runTests 
    tests/test_TastyWorks.cpp
    src/tastyworks/TastyWorks.cpp
    src/config/Config.cpp
)

target_link_libraries(runTests PRIVATE
    GTest::GTest
    GTest::Main
    httplib::httplib
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
)

add_test(NAME tests COMMAND runTests)

# -------------------------
# Debug Info
# -------------------------
message(STATUS "Using libpqxx includes: ${LIBPQXX_INCLUDE_DIRS}")
message(STATUS "Using libpqxx libs: ${LIBPQXX_LIBRARIES}")
