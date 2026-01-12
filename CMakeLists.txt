cmake_minimum_required(VERSION 3.15)
project(mochaFiTrader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# -------------------------
# Source files
# -------------------------
set(SOURCES
    # source files
    src/main.cpp
    src/tastyworks/TastyWorks.cpp
    src/streamer/DX_LinkStreamer.cpp
    src/database/DB_Client.cpp
    src/config/Config.cpp
    src/log/Log.cpp
)

# project headers
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

find_package(fmt CONFIG REQUIRED)

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
    fmt::fmt
)

# -------------------------
# Unit Tests
# -------------------------
enable_testing()
find_package(GTest REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(runUnitTests
    # unit tests
    tests/unit/main.cpp
    tests/unit/test_Config.cpp
    tests/unit/test_DB_Client.cpp
    tests/unit/test_DX_LinkStreamer.cpp
    tests/unit/test_Log.cpp
    tests/unit/test_MarketQuote.cpp
    tests/unit/test_TastyWorks.cpp
    
    # source files
    src/config/Config.cpp
    src/database/DB_Client.cpp
    src/streamer/DX_LinkStreamer.cpp
    src/log/Log.cpp
    src/tastyworks/TastyWorks.cpp
)

target_include_directories(runUnitTests PRIVATE
    ${LIBPQXX_INCLUDE_DIRS}
)

target_link_directories(runUnitTests PRIVATE
    ${LIBPQXX_LIBRARY_DIRS}
)

target_link_libraries(runUnitTests PRIVATE
    GTest::GTest
    GTest::Main
    httplib::httplib
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
    fmt::fmt
    ${LIBPQXX_LIBRARIES}
)

add_test(NAME UnitTests COMMAND runUnitTests)

# -------------------------
# Integration Tests
# -------------------------

add_executable(runIntegrationTests
    # integration tests
    tests/integration/main.cpp
    tests/integration/test_DB_Integration.cpp
    tests/integration/test_LiveStreamIntegrationTest.cpp

    # source files
    src/config/Config.cpp
    src/database/DB_Client.cpp
    src/tastyworks/TastyWorks.cpp
    src/log/Log.cpp
    src/streamer/DX_LinkStreamer.cpp
)

target_include_directories(runIntegrationTests PRIVATE
    ${LIBPQXX_INCLUDE_DIRS}
)

target_link_directories(runIntegrationTests PRIVATE
    ${LIBPQXX_LIBRARY_DIRS}
)

target_link_libraries(runIntegrationTests PRIVATE
    GTest::GTest
    GTest::Main
    httplib::httplib
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
    fmt::fmt
    ${LIBPQXX_LIBRARIES}
)

add_test(NAME IntegrationTests COMMAND runIntegrationTests)

# -------------------------
# Run All Tests
# -------------------------
add_executable(runAllTests
    # all tests
    tests/main.cpp
    
    # unit tests
    tests/unit/test_Config.cpp
    tests/unit/test_DB_Client.cpp
    tests/unit/test_DX_LinkStreamer.cpp
    tests/unit/test_Log.cpp
    tests/unit/test_MarketQuote.cpp
    tests/unit/test_TastyWorks.cpp

    # integration tests
    tests/integration/test_DB_Integration.cpp
    tests/integration/test_LiveStreamIntegrationTest.cpp

    # source files
    src/config/Config.cpp
    src/database/DB_Client.cpp
    src/tastyworks/TastyWorks.cpp
    src/log/Log.cpp
    src/streamer/DX_LinkStreamer.cpp
)

target_include_directories(runAllTests PRIVATE
    ${LIBPQXX_INCLUDE_DIRS}
)

target_link_directories(runAllTests PRIVATE
    ${LIBPQXX_LIBRARY_DIRS}
)

target_link_libraries(runAllTests PRIVATE
    GTest::GTest
    GTest::Main
    httplib::httplib
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
    fmt::fmt
    ${LIBPQXX_LIBRARIES}
)

add_test(NAME AllTests COMMAND runAllTests)

# -------------------------
# Debug Info
# -------------------------
message(STATUS "Using libpqxx includes: ${LIBPQXX_INCLUDE_DIRS}")
message(STATUS "Using libpqxx libs: ${LIBPQXX_LIBRARIES}")
