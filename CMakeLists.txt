cmake_minimum_required(VERSION 3.10)
project(mochaFiTrader)

set(CMAKE_CXX_STANDARD 17)

# Including custom directories
include_directories(${CMAKE_SOURCE_DIR}/src/tastyworks)

# Create the executable
add_executable(mochaFiTrader 
    src/main.cpp
    src/tastyworks/TastyWorks.cpp
)

# packages
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(mochaFiTrader PRIVATE 
    cpr::cpr 
    nlohmann_json::nlohmann_json
)

# unit-tests
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
find_package(httplib CONFIG REQUIRED)

add_executable(runTests 
    tests/test_TastyWorks.cpp
    src/tastyworks/TastyWorks.cpp
)

target_link_libraries(runTests PRIVATE
    ${GTEST_BOTH_LIBRARIES} 
    httplib::httplib pthread
    cpr::cpr
    nlohmann_json::nlohmann_json
)

add_test(NAME tests COMMAND runTests)
