cmake_minimum_required(VERSION 3.20)
project(test-001 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})
add_executable(test-001 main.cpp)
target_include_directories(gateway PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
