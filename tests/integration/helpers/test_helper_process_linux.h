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

#ifndef martianlabs_doba_tests_integration_test_helper_process_linux_h
#define martianlabs_doba_tests_integration_test_helper_process_linux_h

#include <chrono>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

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
    int pipes[2]{-1, -1};
    pid_t pid{-1};
    ~process() {
      if (pid > 0) {
        ::kill(pid, SIGKILL);
        while (::waitpid(pid, nullptr, 0) < 0 && errno == EINTR) {
        }
      }
      if (pipes[1] >= 0) ::close(pipes[1]);
      if (pipes[0] >= 0) ::close(pipes[0]);
    }
  } child;
  const std::filesystem::path program{u8"" DOBA_TEST_HELPER_PROGRAM};
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
  test_helper_result result;
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  bool finished = false;
  while (true) {
    char buffer[4096];
    std::size_t count = 0;
    const ssize_t read = ::read(child.pipes[0], buffer, sizeof(buffer));
    if (read > 0) {
      count = static_cast<std::size_t>(read);
    } else if (read < 0 && errno != EAGAIN && errno != EINTR) {
      fail();
    }
    result.output.append(buffer, count);
    if (finished && count == 0) break;
    if (!finished) {
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
        result.exit_code =
            WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
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
