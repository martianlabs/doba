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

#ifndef martianlabs_doba_tests_helpers_tool_support_h
#define martianlabs_doba_tests_helpers_tool_support_h

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace martianlabs::doba::tests::tools {

namespace fs = std::filesystem;
using clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] require                                                  ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline void require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

inline std::optional<std::string> environment_value(const char* name) {
#ifdef _WIN32
  char* data = nullptr;
  size_t size = 0;
  require(_dupenv_s(&data, &size, name) == 0,
          "Cannot read environment variable");
  if (!data) return std::nullopt;
  std::string result(data);
  std::free(data);
  return result;
#else
  auto data = std::getenv(name);
  return data ? std::optional<std::string>(data) : std::nullopt;
#endif
}

inline std::string read(const fs::path& path) {
  std::ifstream input(path, std::ios::binary);
  require(input.is_open(), "Cannot read: " + path.string());
  std::ostringstream result;
  result << input.rdbuf();
  require(!input.bad(), "Cannot finish reading: " + path.string());
  return result.str();
}

inline void write(const fs::path& path, std::string_view value) {
  if (!path.parent_path().empty()) {
    fs::create_directories(path.parent_path());
  }
  std::ofstream output(path, std::ios::binary);
  output.write(value.data(), static_cast<std::streamsize>(value.size()));
  output.close();
  require(!output.fail(), "Cannot write: " + path.string());
}

inline std::string trim(std::string value) {
  auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};
  return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}

inline std::vector<std::string> lines(const std::string& value) {
  std::vector<std::string> result;
  std::istringstream stream(value);
  for (std::string line; std::getline(stream, line);) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    result.push_back(std::move(line));
  }
  return result;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] temporary_directory                                         ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class temporary_directory {
  fs::path path_;

 public:
  // +=========================================================================+
  // | [>] PUBLIC INTERFACE                                         ( public ) |
  // +-------------------------------------------------------------------------+
  temporary_directory() {
    std::random_device random;
    for (int attempt = 0; attempt != 100; ++attempt) {
      path_ = fs::temp_directory_path() /
              ("doba-native-" + std::to_string(random()) + "-" +
               std::to_string(random()));
      if (fs::create_directory(path_)) return;
    }
    throw std::runtime_error("Cannot create private temporary directory");
  }
  temporary_directory(const temporary_directory&) = delete;
  temporary_directory& operator=(const temporary_directory&) = delete;
  ~temporary_directory() {
    std::error_code error;
    fs::remove_all(path_, error);
  }
  // +=========================================================================+
  // | [>] path                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  const fs::path& path() const { return path_; }
};

inline void clean_diagnostics(const std::string& text) {
  static const std::regex report(
      "(?:ERROR|WARNING|SUMMARY|FATAL):\\s*"
      "(?:Address|Leak|Memory|Thread|UndefinedBehavior)Sanitizer|"
      "(?:Address|Leak|Memory|Thread|UndefinedBehavior)Sanitizer:"
      "\\s*(?:DEADLYSIGNAL|reported [0-9]+ warnings)|runtime error:");
  require(!std::regex_search(text, report),
          "Sanitizer diagnostic in child process:\n" + text);
}

template <class Function>
int guarded(Function function) {
  try {
    function();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

}  // namespace martianlabs::doba::tests::tools

#endif
