cmake_minimum_required(VERSION 3.15)
project(mochaFiTrader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# # -------------------------
# # Static Analysis (clang-tidy)
# # -------------------------
# find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
# if(CLANG_TIDY_EXE)
#     set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
#     message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
# endif()

# # -------------------------
# # Code Formatting (clang-format)
# # -------------------------
# find_program(CLANG_FORMAT_EXE NAMES "clang-format")
# if(CLANG_FORMAT_EXE)
#     file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
#         src/*.cpp src/*.h
#         tests/*.cpp tests/*.h
#     )
#     add_custom_target(format
#         COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_CXX_SOURCE_FILES}
#         COMMENT "Formatting all source files with clang-format"
#     )
# endif()

# # -------------------------
# # Memory Analysis (Valgrind / Memcheck)
# # -------------------------
# find_program(VALGRIND_EXE NAMES "valgrind")
# if(VALGRIND_EXE)
#     set(MEMORYCHECK_COMMAND ${VALGRIND_EXE})
#     set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1")
#     message(STATUS "Valgrind found: ${VALGRIND_EXE}")
# endif()

# -------------------------
# Source files
# -------------------------
set(CORE_SOURCES
    src/tastyworks/TastyWorks.cpp
    src/streamer/DX_LinkStreamer.cpp
    src/config/Config.cpp
    src/log/Log.cpp
    src/algo/algo/Algo.cpp
    src/algo/macd/MACD.cpp
    src/algo/vmacd/VMACD.cpp
    src/algo/rsi/RSI.cpp
    src/algo/ticktype/TickType.cpp
    src/orchestrator/Orchestrator.cpp
    src/order/OrderBuilder.cpp
    src/utils/Utils.cpp
)

add_library(mocha_core STATIC ${CORE_SOURCES})

target_link_libraries(mocha_core PUBLIC
    cpr::cpr
    nlohmann_json::nlohmann_json
    websocketpp::websocketpp
    spdlog::spdlog
    fmt::fmt
    Boost::boost
    Threads::Threads
)

set(MAIN_SOURCES
    src/main.cpp
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
    ${CMAKE_SOURCE_DIR}/src/algo/rsi
    ${CMAKE_SOURCE_DIR}/src/algo/ticktype
    ${CMAKE_SOURCE_DIR}/src/marketquote/
    ${CMAKE_SOURCE_DIR}/src/model/
    ${CMAKE_SOURCE_DIR}/src/orchestrator/
    ${CMAKE_SOURCE_DIR}/src/order/
    ${CMAKE_SOURCE_DIR}/src/utils/
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
add_executable(mochaFiTrader ${MAIN_SOURCES})

target_link_libraries(mochaFiTrader PRIVATE mocha_core)

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
    tests/unit/test_Strategy_MACD.cpp
    tests/unit/test_Strategy_VMACD.cpp
    tests/unit/test_Strategy_RSI.cpp
    tests/unit/test_TradeableAsset.cpp
    tests/unit/test_Algo.cpp
    tests/unit/test_Orchestrator.cpp
    tests/unit/test_OrchestratorRiskMock.cpp
    tests/unit/test_OrchestratorPositionMock.cpp
    tests/unit/test_OrchestratorSellMock.cpp
    tests/unit/test_OrchestratorErrorMock.cpp
    tests/unit/test_WorkflowMock.cpp
    tests/unit/test_SystemIntegrationMock.cpp
    tests/unit/test_LogRotation.cpp
    tests/unit/test_OrderBuilder.cpp
    tests/unit/test_OrderType.cpp
    tests/unit/test_Utils.cpp
    
    # test source files
    tests/test_utils/TestUtils.cpp
)

target_link_libraries(runUnitTests PRIVATE
    mocha_core
    GTest::gtest
    GTest::gmock
    httplib::httplib
)

add_test(NAME UnitTests COMMAND runUnitTests)

# -------------------------
# Integration Tests
# -------------------------

add_executable(runIntegrationTests
    # integration tests
    tests/integration/main.cpp
    tests/integration/test_AuthenticationFlow.cpp
    tests/integration/test_OrderExecutionIntegration.cpp
    tests/integration/test_LiveStreamIntegrationTest.cpp

    # test source files
    tests/test_utils/TestUtils.cpp
)

target_link_libraries(runIntegrationTests PRIVATE
    mocha_core
    GTest::gtest
    GTest::gmock
    httplib::httplib
    Threads::Threads
)

add_test(NAME IntegrationTests COMMAND runIntegrationTests)

# -------------------------
# Run All Tests
# -------------------------
add_executable(runAllTests
    # all tests
    tests/unit/main.cpp
    
    # unit tests
    tests/unit/test_Config.cpp
    tests/unit/test_DX_LinkStreamer.cpp
    tests/unit/test_Log.cpp
    tests/unit/test_MarketQuote.cpp
    tests/unit/test_TastyWorks.cpp
    tests/unit/test_Strategy_MACD.cpp
    tests/unit/test_Strategy_VMACD.cpp
    tests/unit/test_Strategy_RSI.cpp
    tests/unit/test_TradeableAsset.cpp
    tests/unit/test_Algo.cpp
    tests/unit/test_Orchestrator.cpp
    tests/unit/test_OrchestratorRiskMock.cpp
    tests/unit/test_OrchestratorPositionMock.cpp
    tests/unit/test_OrchestratorSellMock.cpp
    tests/unit/test_OrchestratorErrorMock.cpp
    tests/unit/test_WorkflowMock.cpp
    tests/unit/test_SystemIntegrationMock.cpp
    tests/unit/test_LogRotation.cpp
    tests/unit/test_OrderBuilder.cpp
    tests/unit/test_OrderType.cpp
    tests/unit/test_Utils.cpp

    # integration tests
    tests/integration/test_AuthenticationFlow.cpp
    tests/integration/test_OrderExecutionIntegration.cpp
    tests/integration/test_LiveStreamIntegrationTest.cpp

    # test source files
    tests/test_utils/TestUtils.cpp
)

target_link_libraries(runAllTests PRIVATE
    mocha_core
    GTest::gtest
    GTest::gmock
    httplib::httplib
    Threads::Threads
)

add_test(NAME AllTests COMMAND runAllTests)
