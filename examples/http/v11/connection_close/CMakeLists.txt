cmake_minimum_required(
  VERSION 3.20
)

project(
  connection_close
  LANGUAGES CXX
)

add_executable(
  connection_close
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  connection_close
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

