cmake_minimum_required(
  VERSION 3.20
)

project(
  common_byte_storage
  LANGUAGES CXX
)

add_executable(
  common_byte_storage
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  common_byte_storage
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

