cmake_minimum_required(VERSION 3.20)
project(common_cache LANGUAGES CXX)
add_executable(common_cache "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
target_link_libraries(common_cache PRIVATE doba_headers)
