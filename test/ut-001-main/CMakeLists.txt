cmake_minimum_required(VERSION 3.20)

project(ut-001-main LANGUAGES CXX)

add_executable(ut-001-main
  "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
)

target_link_libraries(ut-001-main PRIVATE doba_headers)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}/src"
  FILES "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp"
)
