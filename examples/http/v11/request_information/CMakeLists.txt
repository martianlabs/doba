cmake_minimum_required(
  VERSION 3.20
)

project(
  request_information
  LANGUAGES CXX
)

add_executable(
  request_information
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  request_information
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
