cmake_minimum_required(
  VERSION 3.20
)

project(
  common_task_execution
  LANGUAGES CXX
)

add_executable(
  common_task_execution
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  common_task_execution
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

