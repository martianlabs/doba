cmake_minimum_required(VERSION 3.20)
project(doba_tls_header_guard LANGUAGES CXX)
if(NOT DOBA_INCLUDE_DIR)
  message(FATAL_ERROR "DOBA_INCLUDE_DIR is required")
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/guard.cpp" [=[
#include "https_server.h"
int main() { return sizeof(martianlabs::doba::https_server<>); }
]=])
try_compile(compiled "${CMAKE_CURRENT_BINARY_DIR}/probe"
  "${CMAKE_CURRENT_BINARY_DIR}/guard.cpp"
  CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=20"
    "-DINCLUDE_DIRECTORIES=${DOBA_INCLUDE_DIR}"
  OUTPUT_VARIABLE diagnostics)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/diagnostics.txt" "${diagnostics}")
if(compiled OR NOT diagnostics MATCHES
    "TLS requires the martianlabs::doba_tls CMake target")
  message(FATAL_ERROR "TLS header guard did not reject the plain consumer:\n"
    "${diagnostics}")
endif()
