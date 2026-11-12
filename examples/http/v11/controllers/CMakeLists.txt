cmake_minimum_required(
  VERSION 3.20
)

project(
  controllers
  LANGUAGES CXX
)

add_executable(
  controllers
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  controllers
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
