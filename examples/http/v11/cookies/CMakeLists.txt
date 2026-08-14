cmake_minimum_required(
  VERSION 3.20
)

project(
  cookies
  LANGUAGES CXX
)

add_executable(
  cookies
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  cookies
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
