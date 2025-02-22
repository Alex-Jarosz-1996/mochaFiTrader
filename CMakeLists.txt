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

# package: cpr
find_package(cpr CONFIG REQUIRED)
target_link_libraries(mochaFiTrader PRIVATE cpr::cpr)

# package: nlohmann-json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(mochaFiTrader PRIVATE nlohmann_json::nlohmann_json)
