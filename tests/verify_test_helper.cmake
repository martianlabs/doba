function(
  verify_case
  expected_result
  expected_output
)
  execute_process(
    COMMAND "${TEST_PROGRAM}" ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    TIMEOUT 3
  )
  string(
    APPEND output "${errors}"
  )
  if(
    NOT "${result}" STREQUAL "${expected_result}"
    OR NOT output MATCHES "${expected_output}"
  )
    message(
      FATAL_ERROR
      "Arguments: ${ARGN}\nResult: ${result}\nOutput: ${output}"
    )
  endif()
  set(
    last_output "${output}" PARENT_SCOPE
  )
endfunction()

verify_case(
  0 "probe passes"
  --list
)
if(
  last_output MATCHES "probe body"
)
  message(
    FATAL_ERROR "Listing executed a test"
  )
endif()
verify_case(
  0 "probe body"
  --file test_helper_probe.cpp --name "probe passes"
)
if(
  last_output MATCHES "probe continued"
)
  message(
    FATAL_ERROR "Filtering executed another test"
  )
endif()
verify_case(
  2 "No tests matched"
  --name nonexistent
)
verify_case(
  1 "2 \\+ 2 == 5"
  --name "probe assertion fails"
)
if(
  NOT last_output MATCHES "test_helper_probe.cpp:[0-9]+"
)
  message(
    FATAL_ERROR "Assertion location was not reported"
  )
endif()
verify_case(
  1 "probe standard error"
  --name "probe standard exception"
)
verify_case(
  1 "Unknown exception"
  --name "probe unknown exception"
)
verify_case(
  1 "probe continued"
  --file test_helper_probe.cpp --exclude "probe blocks"
)
verify_case(
  2 "Invalid test arguments"
  --unknown
)
verify_case(
  1 "case: row 7: input=invalid"
  --name "probe row context"
)
execute_process(
  COMMAND "${TEST_PROGRAM}" --name "probe blocks"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE errors
  TIMEOUT 1
)
if(
  NOT result MATCHES "[Tt]imeout"
  OR NOT "${output}${errors}" MATCHES "running.*probe blocks"
)
  message(
    FATAL_ERROR "Blocked test was not identified and timed out: ${result}"
  )
endif()

if(
  VERIFY_LIFETIME
)
  verify_case(
    1 "probe cleanup complete"
    --name "probe failure cleans up active transport"
  )
endif()
