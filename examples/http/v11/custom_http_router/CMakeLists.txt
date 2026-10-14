cmake_minimum_required(
  VERSION 3.20
)

project(
  custom_http_router
  LANGUAGES CXX
)

add_executable(
  custom_http_router
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  custom_http_router
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
