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
  execute_process(COMMAND "${CMAKE_CXX_COMPILER}" --print-resource-dir
    OUTPUT_VARIABLE resource_directory OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
  file(TO_CMAKE_PATH "${runtime_directory}" runtime_directory)
  file(TO_CMAKE_PATH "${resource_directory}" resource_directory)
  get_filename_component(compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)
  # Some LLVM Windows installers report a target-triple runtime directory
  # while shipping compiler-rt under the resource directory's lib/windows.
  # Stay within this compiler's installation, never an unrelated DLL on PATH.
  set(runtime_candidates "${runtime_directory}"
    "${resource_directory}/lib/windows" "${compiler_directory}")
  list(REMOVE_DUPLICATES runtime_candidates)
  foreach(directory IN LISTS runtime_candidates)
    file(GLOB asan_dlls "${directory}/clang_rt.asan_dynamic-*.dll")
    if(asan_dlls)
      break()
    endif()
  endforeach()
  if(NOT asan_dlls)
    string(JOIN "\n  " searched_directories ${runtime_candidates})
    message(FATAL_ERROR
      "Clang Windows ASan runtime not found. Searched:\n  ${searched_directories}")
  endif()
  message(STATUS "Windows ASan runtime for ${target}: ${asan_dlls}")
  foreach(dll IN LISTS asan_dlls)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${dll}" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
  endforeach()
endfunction()
