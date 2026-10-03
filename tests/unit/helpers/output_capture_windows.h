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

#ifndef martianlabs_doba_tests_unit_output_capture_windows_h
#define martianlabs_doba_tests_unit_output_capture_windows_h

#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <io.h>

namespace martianlabs::doba::tests::unit {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] output_capture                                              ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class output_capture {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  output_capture() {
    static std::atomic<std::size_t> sequence{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    path_ = std::filesystem::temp_directory_path() /
            ("doba_console_logger_" + std::to_string(stamp) + "_" +
             std::to_string(sequence.fetch_add(1)));
    file_ = _wfopen(path_.c_str(), L"w+b");
    if (!file_) throw std::runtime_error("Cannot create logger output");
    std::fflush(stdout);
    saved_ = _dup(_fileno(stdout));
    if (saved_ == -1 || _dup2(_fileno(file_), _fileno(stdout)) < 0) {
      if (saved_ != -1) _close(saved_);
      std::fclose(file_);
      std::error_code error;
      std::filesystem::remove(path_, error);
      throw std::runtime_error("Cannot capture logger output");
    }
  }
  output_capture(const output_capture&) = delete;
  ~output_capture() {
    restore();
    std::fclose(file_);
    std::error_code error;
    std::filesystem::remove(path_, error);
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  output_capture& operator=(const output_capture&) = delete;
  // +=========================================================================+
  // | [>] output                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  std::string output() {
    restore();
    std::rewind(file_);
    std::string result;
    char buffer[256];
    for (;;) {
      const std::size_t read = std::fread(buffer, 1, sizeof(buffer), file_);
      result.append(buffer, read);
      if (read < sizeof(buffer)) break;
    }
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] METHODs                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void restore() {
    if (saved_ == -1) return;
    std::fflush(stdout);
    _dup2(saved_, _fileno(stdout));
    _close(saved_);
    saved_ = -1;
  }
  std::filesystem::path path_;
  FILE* file_ = nullptr;
  int saved_ = -1;
};
}  // namespace martianlabs::doba::tests::unit

#endif
