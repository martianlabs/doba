cmake_minimum_required(
  VERSION 3.20
)

project(
  file_download
  LANGUAGES CXX
)

add_executable(
  file_download
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  file_download
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
