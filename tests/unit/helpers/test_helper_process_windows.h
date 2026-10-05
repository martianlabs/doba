//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
//
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_tests_unit_test_helper_process_windows_h
#define martianlabs_doba_tests_unit_test_helper_process_windows_h

#include <chrono>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <windows.h>

namespace martianlabs::doba::tests {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] run_test_helper                                           ( function) |
// +---------------------------------------------------------------------------+
// | This function runs the test helper process with the specified             |
// | command-line arguments and waits for its completion or a timeout.         |
// | It captures the exit code, whether the process timed out, and the output  |
// | written to stdout and stderr.                                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline test_helper_result run_test_helper(
    std::initializer_list<const char*> arguments,
    std::chrono::milliseconds timeout = std::chrono::seconds(3)) {
  // Reap the probe and close its pipes even when output allocation fails.
  struct process {
    HANDLE read{nullptr};
    HANDLE write{nullptr};
    PROCESS_INFORMATION info{};
    ~process() {
      if (info.hProcess != nullptr) {
        if (WaitForSingleObject(info.hProcess, 0) == WAIT_TIMEOUT) {
          TerminateProcess(info.hProcess, 1);
          WaitForSingleObject(info.hProcess, INFINITE);
        }
        CloseHandle(info.hProcess);
      }
      if (info.hThread != nullptr) CloseHandle(info.hThread);
      if (write != nullptr) CloseHandle(write);
      if (read != nullptr) CloseHandle(read);
    }
  } child;
  const std::filesystem::path program{u8"" DOBA_TEST_HELPER_PROGRAM};
  const auto fail = []() {
    throw std::system_error(static_cast<int>(GetLastError()),
                            std::system_category(), "test helper process");
  };
  std::wstring command;
  const auto append_argument = [&](const std::wstring& value) {
    if (!command.empty()) command += L' ';
    command += L'"';
    std::size_t slashes = 0;
    for (wchar_t character : value) {
      if (character == L'\\') {
        slashes++;
        continue;
      }
      command.append(character == L'"' ? slashes * 2 + 1 : slashes, L'\\');
      command += character;
      slashes = 0;
    }
    command.append(slashes * 2, L'\\');
    command += L'"';
  };
  append_argument(program.native());
  for (const char* argument : arguments) {
    const std::string value{argument};
    append_argument(std::wstring{value.begin(), value.end()});
  }
  SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
  if (!CreatePipe(&child.read, &child.write, &security, 0)) fail();
  if (!SetHandleInformation(child.read, HANDLE_FLAG_INHERIT, 0)) fail();
  STARTUPINFOW startup{};
  startup.cb = sizeof(startup);
  startup.dwFlags = STARTF_USESTDHANDLES;
  startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  startup.hStdOutput = child.write;
  startup.hStdError = child.write;
  if (!CreateProcessW(program.c_str(), command.data(), nullptr, nullptr, TRUE,
                      CREATE_NO_WINDOW, nullptr, nullptr, &startup,
                      &child.info)) {
    fail();
  }
  CloseHandle(child.write);
  child.write = nullptr;
  test_helper_result result;
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  bool finished = false;
  while (true) {
    char buffer[4096];
    std::size_t count = 0;
    DWORD available = 0;
    if (!PeekNamedPipe(child.read, nullptr, 0, nullptr, &available, nullptr)) {
      if (GetLastError() != ERROR_BROKEN_PIPE) fail();
    } else if (available != 0) {
      DWORD read = 0;
      if (!ReadFile(child.read, buffer, sizeof(buffer), &read, nullptr)) {
        fail();
      }
      count = read;
    }
    result.output.append(buffer, count);
    if (finished && count == 0) break;
    if (!finished) {
      const DWORD status = WaitForSingleObject(child.info.hProcess, 0);
      if (status == WAIT_FAILED) fail();
      finished = status == WAIT_OBJECT_0;
      if (!finished && std::chrono::steady_clock::now() >= deadline) {
        if (!TerminateProcess(child.info.hProcess, 1)) fail();
        if (WaitForSingleObject(child.info.hProcess, INFINITE) !=
            WAIT_OBJECT_0) {
          fail();
        }
        result.timed_out = true;
        finished = true;
      }
      if (finished) {
        DWORD code = 0;
        if (!GetExitCodeProcess(child.info.hProcess, &code)) fail();
        result.exit_code = static_cast<int>(code);
      }
    }
    if (!finished && count == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  return result;
}
}  // namespace martianlabs::doba::tests

#endif
