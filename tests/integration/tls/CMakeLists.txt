set(_tls_wire_runners exercise processing_wire)
if(TARGET https_hello_world AND TARGET tls_echo)
  list(APPEND _tls_wire_runners examples_wire)
endif()
foreach(runner IN LISTS _tls_wire_runners)
  add_executable(doba_tls_${runner} "${runner}.cpp")
  target_link_libraries(doba_tls_${runner} PRIVATE
    doba_native_tools OpenSSL::SSL)
  target_compile_definitions(doba_tls_${runner} PRIVATE DOBA_TOOL_TLS=1)
  if(WIN32)
    target_link_libraries(doba_tls_${runner} PRIVATE ws2_32)
  endif()
endforeach()
add_executable(doba_tls_fixture fixture.cpp)
add_dependencies(doba_tls_exercise doba_tls_fixture)
add_dependencies(doba_tls_processing_wire doba_tls_fixture)
target_link_libraries(doba_tls_fixture PRIVATE martianlabs::doba_tls)
add_test(NAME doba_tls_tests COMMAND doba_tls_exercise
  "$<TARGET_FILE:doba_tls_fixture>"
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
  add_dependencies(doba_tls_examples_wire https_hello_world tls_echo)
  add_test(NAME doba_tls_examples_tests COMMAND doba_tls_examples_wire
    "$<TARGET_FILE:https_hello_world>" "$<TARGET_FILE:tls_echo>"
    "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
  set_tests_properties(doba_tls_examples_tests PROPERTIES TIMEOUT 45)
endif()

add_executable(doba_tls_processing processing.cpp)
target_link_libraries(doba_tls_processing PRIVATE martianlabs::doba_tls)
add_test(NAME doba_tls_processing_tests COMMAND doba_tls_processing
  "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
set_tests_properties(doba_tls_processing_tests PROPERTIES TIMEOUT 45)

add_test(NAME doba_tls_processing_wire_tests COMMAND doba_tls_processing_wire
  "$<TARGET_FILE:doba_tls_fixture>"
  "${CMAKE_CURRENT_LIST_DIR}/cert.pem" "${CMAKE_CURRENT_LIST_DIR}/key.pem")
set_tests_properties(doba_tls_processing_wire_tests PROPERTIES TIMEOUT 90)
