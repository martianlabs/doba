cmake_minimum_required(
  VERSION 3.20
)

project(
  http_processing_turns
  LANGUAGES CXX
)

add_executable(
  http_processing_turns
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  http_processing_turns
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

