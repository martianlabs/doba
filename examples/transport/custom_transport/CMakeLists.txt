cmake_minimum_required(
  VERSION 3.20
)

project(
  custom_transport
  LANGUAGES CXX
)

add_executable(
  custom_transport
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  custom_transport
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
