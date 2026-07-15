cmake_minimum_required(VERSION 3.20)

project(ut-002-request-plus-contract LANGUAGES CXX)

add_executable(ut-002-request-plus-contract
  "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
)

target_link_libraries(ut-002-request-plus-contract PRIVATE doba_headers)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}/src"
  FILES "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
)
