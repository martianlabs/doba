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

#ifndef martianlabs_doba_tests_test_helper_process_h
#define martianlabs_doba_tests_test_helper_process_h

#include <chrono>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace martianlabs::doba::tests {
// +===========================================================================+
// | [>] test_helper_result                                         ( struct ) |
// +===========================================================================+
struct test_helper_result {
  int exit_code{-1};
  bool timed_out{false};
  std::string output;
};

// +===========================================================================+
// | [>] run_test_helper                                            ( method ) |
// +===========================================================================+
inline test_helper_result run_test_helper(
    std::initializer_list<const char*> arguments,
    std::chrono::milliseconds timeout = std::chrono::seconds(3)) {
  // Reap the probe and close its pipes even when output allocation fails.
  struct process {
#ifdef _WIN32
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
#else
    int pipes[2]{-1, -1};
    pid_t pid{-1};
    ~process() {
      if (pid > 0) {
        ::kill(pid, SIGKILL);
        while (::waitpid(pid, nullptr, 0) < 0 && errno == EINTR) {}
      }
      if (pipes[1] >= 0) ::close(pipes[1]);
      if (pipes[0] >= 0) ::close(pipes[0]);
    }
#endif
  } child;
  const std::filesystem::path program{u8"" DOBA_TEST_HELPER_PROGRAM};
#ifdef _WIN32
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
      command.append(character == L'"' ? slashes * 2 + 1 : slashes,
                     L'\\');
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
  if (!CreateProcessW(program.c_str(), command.data(), nullptr, nullptr,
                      TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup,
                      &child.info)) {
    fail();
  }
  CloseHandle(child.write);
  child.write = nullptr;
#else
  const auto fail = []() {
    throw std::system_error(errno, std::generic_category(),
                            "test helper process");
  };
  std::vector<char*> argv{const_cast<char*>(program.c_str())};
  for (const char* argument : arguments) {
    argv.push_back(const_cast<char*>(argument));
  }
  argv.push_back(nullptr);
  if (::pipe2(child.pipes, O_CLOEXEC) != 0) fail();
  if (::fcntl(child.pipes[0], F_SETFL, O_NONBLOCK) == -1) fail();
  child.pid = ::fork();
  if (child.pid < 0) fail();
  if (child.pid == 0) {
    if (::dup2(child.pipes[1], STDOUT_FILENO) == -1 ||
        ::dup2(child.pipes[1], STDERR_FILENO) == -1) {
      ::_exit(127);
    }
    ::close(child.pipes[0]);
    ::close(child.pipes[1]);
    ::execv(program.c_str(), argv.data());
    ::_exit(127);
  }
  ::close(child.pipes[1]);
  child.pipes[1] = -1;
#endif
  test_helper_result result;
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  bool finished = false;
  while (true) {
    char buffer[4096];
    std::size_t count = 0;
#ifdef _WIN32
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
#else
    const ssize_t read = ::read(child.pipes[0], buffer, sizeof(buffer));
    if (read > 0) {
      count = static_cast<std::size_t>(read);
    } else if (read < 0 && errno != EAGAIN && errno != EINTR) {
      fail();
    }
#endif
    result.output.append(buffer, count);
    if (finished && count == 0) break;
    if (!finished) {
#ifdef _WIN32
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
#else
      int status = 0;
      pid_t waited = ::waitpid(child.pid, &status, WNOHANG);
      if (waited < 0 && errno != EINTR) fail();
      if (waited == 0 && std::chrono::steady_clock::now() >= deadline) {
        if (::kill(child.pid, SIGKILL) != 0) fail();
        do {
          waited = ::waitpid(child.pid, &status, 0);
        } while (waited < 0 && errno == EINTR);
        if (waited < 0) fail();
        result.timed_out = true;
      }
      if (waited > 0) {
        child.pid = -1;
        finished = true;
        result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status)
                                            : 128 + WTERMSIG(status);
      }
#endif
    }
    if (!finished && count == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  return result;
}
}  // namespace martianlabs::doba::tests

#endif
