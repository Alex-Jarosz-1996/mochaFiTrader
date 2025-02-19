cmake_minimum_required(VERSION 3.10)
project(mochaFiTrader)

set(CMAKE_CXX_STANDARD 17)

# Create the executable
add_executable(mochaFiTrader src/main.cpp)

find_package(cpr CONFIG REQUIRED)
target_link_libraries(mochaFiTrader PRIVATE cpr::cpr)