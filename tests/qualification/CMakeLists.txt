cmake_minimum_required(VERSION 3.20)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  project(doba_qualification LANGUAGES CXX)
endif()

# Standalone bootstrap stays uninstrumented. Root builds also qualify these
# tools under the selected compiler and sanitizer without exporting any API.
find_package(Threads REQUIRED)
add_library(doba_native_tools INTERFACE)
target_compile_features(doba_native_tools INTERFACE cxx_std_20)
target_link_libraries(doba_native_tools INTERFACE Threads::Threads)
target_compile_definitions(doba_native_tools INTERFACE
  DOBA_TOOL_SOURCE_ROOT="${CMAKE_CURRENT_LIST_DIR}/../..")
if(MSVC)
  target_compile_options(doba_native_tools INTERFACE /W4 /WX /utf-8)
  target_compile_definitions(doba_native_tools INTERFACE
    NOMINMAX)
else()
  target_compile_options(doba_native_tools INTERFACE
    -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(doba_coverage coverage.cpp)
target_link_libraries(doba_coverage PRIVATE doba_native_tools)
add_executable(doba_campaign campaign.cpp)
target_link_libraries(doba_campaign PRIVATE doba_native_tools)

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  add_executable(doba_qualification_controls qualification_controls.cpp)
  target_link_libraries(doba_qualification_controls PRIVATE doba_native_tools)
  add_test(NAME doba_qualification_tests COMMAND doba_qualification_controls)
  set_tests_properties(doba_qualification_tests PROPERTIES TIMEOUT 30)

  add_executable(doba_tool_controls ../helpers/tool_controls.cpp)
  target_link_libraries(doba_tool_controls PRIVATE doba_native_tools)
  add_test(NAME doba_test_diagnostics_tests COMMAND doba_tool_controls)
  set_tests_properties(doba_test_diagnostics_tests PROPERTIES TIMEOUT 30)
endif()
