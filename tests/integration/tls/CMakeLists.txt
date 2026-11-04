find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
add_executable(doba_tls_fixture fixture.cpp)
target_link_libraries(doba_tls_fixture PRIVATE martianlabs::doba_tls)
add_test(NAME doba_tls_tests COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_CURRENT_LIST_DIR}/exercise.py" "$<TARGET_FILE:doba_tls_fixture>"
  "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
set_tests_properties(doba_tls_tests PROPERTIES TIMEOUT 120)
add_executable(doba_tls_core core.cpp)
target_link_libraries(doba_tls_core PRIVATE martianlabs::doba_tls)
add_test(NAME doba_tls_core_tests COMMAND doba_tls_core
  "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
set_tests_properties(doba_tls_core_tests PROPERTIES TIMEOUT 30)

add_executable(doba_tls_configuration configuration.cpp)
target_link_libraries(doba_tls_configuration PRIVATE martianlabs::doba_tls)
add_test(NAME doba_tls_configuration_tests COMMAND doba_tls_configuration
  "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
set_tests_properties(doba_tls_configuration_tests PROPERTIES TIMEOUT 30)

if(TARGET https_hello_world AND TARGET tls_echo)
  add_test(NAME doba_tls_examples_tests COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_LIST_DIR}/examples.py"
    "$<TARGET_FILE:https_hello_world>" "$<TARGET_FILE:tls_echo>"
    "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
  set_tests_properties(doba_tls_examples_tests PROPERTIES TIMEOUT 45)
endif()

add_test(NAME doba_tls_header_guard_tests COMMAND "${CMAKE_COMMAND}"
  -S "${CMAKE_CURRENT_LIST_DIR}/../../package/tls_guard"
  -B "${CMAKE_CURRENT_BINARY_DIR}/header-guard"
  -G "${CMAKE_GENERATOR}"
  "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
  "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
  "-DDOBA_INCLUDE_DIR=${DOBA_INCLUDE_DIR}")
set_tests_properties(doba_tls_header_guard_tests PROPERTIES TIMEOUT 45)

add_executable(doba_tls_processing processing.cpp)
target_link_libraries(doba_tls_processing PRIVATE martianlabs::doba_tls)
add_test(NAME doba_tls_processing_tests COMMAND doba_tls_processing
  "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
set_tests_properties(doba_tls_processing_tests PROPERTIES TIMEOUT 45)

add_test(NAME doba_tls_processing_wire_tests COMMAND "${Python3_EXECUTABLE}"
  "${CMAKE_CURRENT_LIST_DIR}/processing.py" "$<TARGET_FILE:doba_tls_fixture>"
  "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
set_tests_properties(doba_tls_processing_wire_tests PROPERTIES TIMEOUT 90)
