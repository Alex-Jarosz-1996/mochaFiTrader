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
    src/config/Config.cpp
    src/log/Log.cpp
    src/algo/algo/Algo.cpp
    src/algo/macd/MACD.cpp
    src/algo/vmacd/VMACD.cpp
    src/algo/ticktype/TickType.cpp
)

# project headers
include_directories(
    ${CMAKE_SOURCE_DIR}/src/tastyworks
    ${CMAKE_SOURCE_DIR}/src/streamer
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/log
    ${CMAKE_SOURCE_DIR}/src/algo
    ${CMAKE_SOURCE_DIR}/src/algo/algo
    ${CMAKE_SOURCE_DIR}/src/algo/macd
    ${CMAKE_SOURCE_DIR}/src/algo/vmacd
    ${CMAKE_SOURCE_DIR}/src/algo/ticktype
)

# -------------------------
# Dependencies
# -------------------------
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# -------------------------
# Executable
# -------------------------
add_executable(mochaFiTrader ${SOURCES})

target_link_libraries(mochaFiTrader PRIVATE 
    cpr::cpr 
    nlohmann_json::nlohmann_json
    websocketpp::websocketpp
    Boost::boost
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
)

# -------------------------
# Unit Tests
# -------------------------
enable_testing()
find_package(GTest REQUIRED)
find_package(httplib CONFIG REQUIRED)

add_executable(runUnitTests
    # unit tests
    tests/unit/main.cpp
    tests/unit/test_Config.cpp
    tests/unit/test_DX_LinkStreamer.cpp
    tests/unit/test_Log.cpp
    tests/unit/test_MarketQuote.cpp
    tests/unit/test_TastyWorks.cpp
    tests/unit/test_Strategy_Algo.cpp
    tests/unit/test_Strategy_MACD.cpp
    tests/unit/test_Strategy_VMACD.cpp
    
    # source files
    src/config/Config.cpp
    src/streamer/DX_LinkStreamer.cpp
    src/log/Log.cpp
    src/tastyworks/TastyWorks.cpp
    src/algo/algo/Algo.cpp
    src/algo/macd/MACD.cpp
    src/algo/vmacd/VMACD.cpp
    src/algo/ticktype/TickType.cpp
    
    # test source files
    tests/test_utils/TestUtils.cpp
)

target_link_libraries(runUnitTests PRIVATE
    GTest::GTest
    GTest::Main
    httplib::httplib
    Threads::Threads
    cpr::cpr
    nlohmann_json::nlohmann_json
    fmt::fmt
)

add_test(NAME UnitTests COMMAND runUnitTests)

# -------------------------
# Integration Tests
# -------------------------

add_executable(runIntegrationTests
    # integration tests
    tests/integration/main.cpp
    tests/integration/test_LiveStreamIntegrationTest.cpp

    # source files
    src/config/Config.cpp
    src/tastyworks/TastyWorks.cpp
    src/log/Log.cpp
    src/streamer/DX_LinkStreamer.cpp
    src/algo/algo/Algo.cpp
    src/algo/macd/MACD.cpp
    src/algo/vmacd/VMACD.cpp
    src/algo/ticktype/TickType.cpp

    # test source files
    tests/test_utils/TestUtils.cpp
)

target_link_libraries(runIntegrationTests PRIVATE
    GTest::GTest
    GTest::Main
    httplib::httplib
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
    fmt::fmt
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
    tests/unit/test_DX_LinkStreamer.cpp
    tests/unit/test_Log.cpp
    tests/unit/test_MarketQuote.cpp
    tests/unit/test_TastyWorks.cpp
    tests/unit/test_Strategy_Algo.cpp
    tests/unit/test_Strategy_MACD.cpp
    tests/unit/test_Strategy_VMACD.cpp

    # integration tests
    tests/integration/test_LiveStreamIntegrationTest.cpp

    # source files
    src/config/Config.cpp
    src/tastyworks/TastyWorks.cpp
    src/log/Log.cpp
    src/streamer/DX_LinkStreamer.cpp
    src/algo/algo/Algo.cpp
    src/algo/macd/MACD.cpp
    src/algo/vmacd/VMACD.cpp
    src/algo/ticktype/TickType.cpp
    
    # test source files
    tests/test_utils/TestUtils.cpp
)

target_link_libraries(runAllTests PRIVATE
    GTest::GTest
    GTest::Main
    httplib::httplib
    pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
    fmt::fmt
)

add_test(NAME AllTests COMMAND runAllTests)
