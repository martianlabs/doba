cmake_minimum_required(
  VERSION 3.20
)

project(
  expect_continue
  LANGUAGES CXX
)

add_executable(
  expect_continue
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

target_link_libraries(
  expect_continue
  PRIVATE
  doba_headers
)

source_group(
  TREE "${CMAKE_CURRENT_LIST_DIR}"
  FILES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

