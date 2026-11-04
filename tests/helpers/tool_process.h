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
// Copyright 2026 martianLabs
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

#ifndef martianlabs_doba_tests_helpers_tool_process_h
#define martianlabs_doba_tests_helpers_tool_process_h

#include "tool_support.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace martianlabs::doba::tests::tools {

// +---------------------------------------------------------------------------+
// | [>] process                                                    ( class ) |
// +---------------------------------------------------------------------------+
// Output goes to private files: even a verbose sanitizer child cannot fill a
// pipe while its parent waits. Destruction always reaps an unfinished child.
class process {
  temporary_directory directory_;
  int code_{-1};
  bool finished_{};
#ifdef _WIN32
  HANDLE child_{};
  HANDLE input_{};

  static std::wstring wide(const std::string& value) {
    int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
      value.data(), static_cast<int>(value.size()), nullptr, 0);
    require(size > 0 || value.empty(), "Invalid UTF-8 process argument");
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
      static_cast<int>(value.size()), result.data(), size);
    return result;
  }
  static std::wstring quote(const std::string& argument) {
    std::wstring result = L"\"";
    unsigned slashes = 0;
    for (wchar_t character : wide(argument)) {
      if (character == L'\\') { ++slashes; continue; }
      result.append(character == L'"' ? slashes * 2 + 1 : slashes, L'\\');
      result += character;
      slashes = 0;
    }
    result.append(slashes * 2, L'\\');
    return result + L'"';
  }
#else
  pid_t child_{-1};
  int input_{-1};
#endif

 public:
  // +-------------------------------------------------------------------------+
  // | [>] PUBLIC INTERFACE                                       ( public ) |
  // +-------------------------------------------------------------------------+
  using environment = std::map<std::string, std::string>;
  process(const std::vector<std::string>& command,
          const environment& overrides = {},
          const std::vector<std::string>& unset = {}, bool console = false) {
    require(!command.empty(), "Empty child command");
    write(directory_.path() / "stdout", "");
    write(directory_.path() / "stderr", "");
#ifdef _WIN32
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE reader{};
    require(CreatePipe(&reader, &input_, &security, 0) != 0,
      "Cannot create child stdin");
    SetHandleInformation(input_, HANDLE_FLAG_INHERIT, 0);
    HANDLE output = CreateFileW((directory_.path() / "stdout").c_str(),
      GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE error = CreateFileW((directory_.path() / "stderr").c_str(),
      GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    std::map<std::wstring, std::wstring> variables;
    auto block = GetEnvironmentStringsW();
    require(block != nullptr, "Cannot read child environment");
    for (auto entry = block; *entry; entry += wcslen(entry) + 1) {
      std::wstring item(entry);
      auto equal = item.find(L'=', item[0] == L'=' ? 1 : 0);
      if (equal != std::wstring::npos) {
        variables[item.substr(0, equal)] = item.substr(equal + 1);
      }
    }
    FreeEnvironmentStringsW(block);
    for (const auto& key : unset) variables.erase(wide(key));
    for (const auto& [key, value] : overrides) {
      variables[wide(key)] = wide(value);
    }
    std::wstring environment_block;
    for (const auto& [key, value] : variables) {
      environment_block += key + L'=' + value;
      environment_block += L'\0';
    }
    environment_block += L'\0';
    std::wstring arguments;
    for (const auto& argument : command) {
      if (!arguments.empty()) arguments += L' ';
      arguments += quote(argument);
    }
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    startup.hStdInput = reader;
    startup.hStdOutput = output;
    startup.hStdError = error;
    PROCESS_INFORMATION information{};
    BOOL created = CreateProcessW(nullptr, arguments.data(), nullptr, nullptr,
      TRUE, (console ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW) |
        CREATE_UNICODE_ENVIRONMENT,
      environment_block.data(), nullptr, &startup, &information);
    CloseHandle(reader);
    CloseHandle(output);
    CloseHandle(error);
    if (!created) {
      CloseHandle(input_);
      input_ = nullptr;
      throw std::runtime_error("Cannot start child: " + command.front());
    }
    CloseHandle(information.hThread);
    child_ = information.hProcess;
#else
    (void)console;
    std::map<std::string, std::string> variables;
    for (char** entry = environ; *entry; ++entry) {
      std::string item(*entry);
      auto equal = item.find('=');
      variables[item.substr(0, equal)] = item.substr(equal + 1);
    }
    for (const auto& key : unset) variables.erase(key);
    for (const auto& [key, value] : overrides) variables[key] = value;
    std::vector<std::string> storage;
    for (const auto& [key, value] : variables) {
      storage.push_back(key + '=' + value);
    }
    std::vector<char*> environment_block, arguments;
    for (auto& item : storage) environment_block.push_back(item.data());
    environment_block.push_back(nullptr);
    for (const auto& item : command) arguments.push_back(
      const_cast<char*>(item.c_str()));
    arguments.push_back(nullptr);
    // Resolve PATH in the parent. The child only calls async-signal-safe APIs.
    fs::path executable = command.front();
    if (command.front().find('/') == std::string::npos) {
      std::istringstream paths(variables["PATH"]);
      for (std::string path; std::getline(paths, path, ':');) {
        auto candidate = fs::path(path) / executable;
        if (access(candidate.c_str(), X_OK) == 0) {
          executable = candidate;
          break;
        }
      }
    }
    int pipes[2];
    require(pipe2(pipes, O_CLOEXEC) == 0, "Cannot create child stdin");
    int output = open((directory_.path() / "stdout").c_str(),
      O_WRONLY | O_CLOEXEC);
    int error = open((directory_.path() / "stderr").c_str(),
      O_WRONLY | O_CLOEXEC);
    if (output < 0 || error < 0) {
      close(pipes[0]); close(pipes[1]);
      if (output >= 0) close(output);
      if (error >= 0) close(error);
      throw std::runtime_error("Cannot redirect child output");
    }
    child_ = fork();
    if (child_ == 0) {
      dup2(pipes[0], STDIN_FILENO);
      dup2(output, STDOUT_FILENO);
      dup2(error, STDERR_FILENO);
      close(pipes[0]); close(pipes[1]); close(output); close(error);
      execve(executable.c_str(), arguments.data(), environment_block.data());
      _exit(127);
    }
    close(pipes[0]); close(output); close(error);
    input_ = pipes[1];
    if (child_ < 0) {
      close(input_);
      input_ = -1;
      throw std::runtime_error("Cannot fork child");
    }
    // Native peers also use MSG_NOSIGNAL; stdin writes need this safeguard.
    signal(SIGPIPE, SIG_IGN);
#endif
  }
  process(const process&) = delete;
  process& operator=(const process&) = delete;
  ~process() {
    if (!finished_) {
      kill();
      try { wait(5s); } catch (...) {}
    }
#ifdef _WIN32
    if (input_) CloseHandle(input_);
    if (child_) CloseHandle(child_);
#else
    if (input_ >= 0) close(input_);
#endif
  }
  bool poll() {
    if (finished_) return true;
#ifdef _WIN32
    if (WaitForSingleObject(child_, 0) == WAIT_TIMEOUT) return false;
    DWORD value{};
    require(GetExitCodeProcess(child_, &value) != 0, "Cannot query child exit");
    code_ = static_cast<int>(value);
#else
    int status{};
    pid_t result;
    do { result = waitpid(child_, &status, WNOHANG); }
    while (result < 0 && errno == EINTR);
    require(result >= 0, "Cannot reap child");
    if (!result) return false;
    code_ = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
#endif
    finished_ = true;
    return true;
  }
  int wait(clock::duration timeout = 10s) {
    auto deadline = clock::now() + timeout;
    while (!poll()) {
      require(clock::now() < deadline, "Child deadline exceeded");
      std::this_thread::sleep_for(1ms);
    }
    return code_;
  }
  void terminate() noexcept {
    if (finished_) return;
#ifdef _WIN32
    if (child_) TerminateProcess(child_, 1);
#else
    if (child_ > 0) ::kill(child_, SIGTERM);
#endif
  }
  void kill() noexcept {
    if (finished_) return;
#ifdef _WIN32
    if (child_) TerminateProcess(child_, 1);
#else
    if (child_ > 0) ::kill(child_, SIGKILL);
#endif
  }
  void send(std::string_view text) {
    while (!text.empty()) {
#ifdef _WIN32
      DWORD size{};
      require(WriteFile(input_, text.data(),
        static_cast<DWORD>(text.size()), &size, nullptr) != 0 && size,
        "Cannot write child stdin");
#else
      auto size = ::write(input_, text.data(), text.size());
      if (size < 0 && errno == EINTR) continue;
      require(size > 0, "Cannot write child stdin");
#endif
      text.remove_prefix(static_cast<size_t>(size));
    }
  }
  static std::string text(std::string value) {
    size_t output = 0;
    for (size_t input = 0; input < value.size(); ++input) {
      if (value[input] == '\r' && input + 1 < value.size() &&
          value[input + 1] == '\n') continue;
      value[output++] = value[input];
    }
    value.resize(output);
    return value;
  }
  fs::path output_path() const { return directory_.path() / "stdout"; }
  fs::path error_path() const { return directory_.path() / "stderr"; }
  std::string output() const {
    return text(read(directory_.path() / "stdout"));
  }
  std::string errors() const {
    return text(read(directory_.path() / "stderr"));
  }
  void expect_output(std::string_view text, clock::duration timeout = 10s) {
    auto deadline = clock::now() + timeout;
    while (output().find(text) == std::string::npos) {
      require(!poll(), "Child exited before readiness: " + errors());
      require(clock::now() < deadline, "Child readiness deadline exceeded");
      std::this_thread::sleep_for(1ms);
    }
  }
};

struct process_result {
  int code;
  std::string output;
  std::string errors;
};

inline process_result run(const std::vector<std::string>& command,
  clock::duration timeout = 180s,
  const process::environment& environment = {}) {
  process child(command, environment);
  int code = child.wait(timeout);
  return {code, child.output(), child.errors()};
}

} // namespace martianlabs::doba::tests::tools

#endif
