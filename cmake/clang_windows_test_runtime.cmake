# Keep native Clang verification executables independent of an unrelated
# Visual Studio ASan DLL earlier on PATH. Instrumentation remains target-local.
function(doba_clang_windows_test_runtime target)
  if(NOT WIN32)
    return()
  endif()
  set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)
  target_link_options(${target} PRIVATE -Xlinker /INCREMENTAL:NO)
  execute_process(COMMAND "${CMAKE_CXX_COMPILER}" --print-runtime-dir
    OUTPUT_VARIABLE runtime_directory OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
  file(TO_CMAKE_PATH "${runtime_directory}" runtime_directory)
  file(GLOB asan_dlls "${runtime_directory}/clang_rt.asan_dynamic-*.dll")
  if(NOT asan_dlls)
    message(FATAL_ERROR "Clang Windows ASan runtime not found in ${runtime_directory}")
  endif()
  foreach(dll IN LISTS asan_dlls)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${dll}" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
  endforeach()
endfunction()
