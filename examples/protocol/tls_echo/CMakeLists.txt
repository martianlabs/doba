cmake_minimum_required(
  VERSION 3.20
)

project(
  tls_echo
  LANGUAGES CXX
)

add_executable(
  tls_echo
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  tls_echo
  PRIVATE
  martianlabs::doba_tls
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
