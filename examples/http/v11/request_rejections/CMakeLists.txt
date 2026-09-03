cmake_minimum_required(
  VERSION 3.20
)

project(
  request_rejections
  LANGUAGES CXX
)

add_executable(
  request_rejections
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  request_rejections
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

