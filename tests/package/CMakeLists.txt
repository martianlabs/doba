cmake_minimum_required(
  VERSION 3.20
)

project(
  doba_package_consumer
  LANGUAGES CXX
)

find_package(
  doba
  CONFIG
  REQUIRED
)

add_executable(
  doba_package_consumer
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)
target_link_libraries(
  doba_package_consumer
  PRIVATE
  martianlabs::doba
)
