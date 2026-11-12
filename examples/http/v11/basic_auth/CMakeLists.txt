cmake_minimum_required(
  VERSION 3.20
)

project(
  basic_auth
  LANGUAGES CXX
)

add_executable(
  basic_auth
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  basic_auth
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
