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

#include "test_helper.h"

#include <cstddef>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>

#include "common/console_logger.h"

namespace martianlabs::doba::tests::unit {
namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] test_case                                                  ( struct ) |
// +---------------------------------------------------------------------------+
// | Test case data.                                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct test_case {
  std::string_view file;
  int line;
  std::string_view name;
  test_helper::test_t test;
};
// +===========================================================================+
// | [>] relative_test_path                                         ( method ) |
// +===========================================================================+
std::string_view relative_test_path(std::string_view file) {
  constexpr std::string_view windows_marker = "\\tests\\unit\\";
  constexpr std::string_view unix_marker = "/tests/unit/";
  std::size_t pos = file.rfind(windows_marker);
  if (pos == std::string_view::npos) pos = file.rfind(unix_marker);
  if (pos != std::string_view::npos) file.remove_prefix(pos + 1);
  return file;
}
// +===========================================================================+
// | [>] tests                                                      ( method ) |
// +===========================================================================+
std::vector<test_case>& tests() {
  static std::vector<test_case> value;
  return value;
}
// +===========================================================================+
// | [>] failures                                                   ( method ) |
// +===========================================================================+
std::size_t& failures() {
  static std::size_t value = 0;
  return value;
}
// +===========================================================================+
// | [>] context                                                    ( method ) |
// +===========================================================================+
std::string& context() {
  static std::string value;
  return value;
}
}  // namespace

// +===========================================================================+
// | [>] add                                                        ( method ) |
// +===========================================================================+
bool test_helper::add(std::string_view file, int line, std::string_view name,
                      test_t test) {
  tests().push_back({relative_test_path(file), line, name, test});
  return true;
}
// +===========================================================================+
// | [>] expect                                                     ( method ) |
// +===========================================================================+
bool test_helper::expect(bool condition, std::string_view expression,
                         std::string_view file, int line) {
  if (condition) return true;
  failures()++;
  file = relative_test_path(file);
  std::fprintf(stderr, "%.*s:%d - assertion failed: %.*s\n",
               static_cast<int>(file.size()), file.data(), line,
               static_cast<int>(expression.size()), expression.data());
  if (!context().empty()) {
    std::fprintf(stderr, "case: %s\n", context().c_str());
  }
  std::fflush(stderr);
  return false;
}
// +===========================================================================+
// | [>] set_context                                                ( method ) |
// +===========================================================================+
void test_helper::set_context(std::string_view value) {
  context().assign(value);
}
// +===========================================================================+
// | [>] run                                                        ( method ) |
// +===========================================================================+
int test_helper::run(int argc, char** argv) {
  bool list = false;
  std::string file;
  std::string_view name;
  std::vector<std::string> exclude;
  for (int i = 1; i < argc; i++) {
    const std::string_view argument{argv[i]};
    if (argument == "--list") {
      list = true;
    } else if ((argument == "--file" || argument == "--name" ||
                argument == "--exclude") && i + 1 < argc) {
      const std::string_view value{argv[++i]};
      if (argument == "--file") file = value;
      if (argument == "--name") name = value;
      if (argument == "--exclude") exclude.emplace_back(value);
    } else {
      std::fputs("Invalid test arguments\n", stderr);
      return 2;
    }
  }
  for (char& value : file) {
    if (value == '\\') value = '/';
  }
  for (std::string& value : exclude) {
    const std::size_t separator = value.find("::");
    if (separator == std::string::npos) continue;
    for (std::size_t i = 0; i < separator; i++) {
      if (value[i] == '\\') value[i] = '/';
    }
  }
  const auto is_excluded = [&](const test_case& test) {
    std::string normalized{test.file};
    for (char& value : normalized) {
      if (value == '\\') value = '/';
    }
    for (const std::string& value : exclude) {
      const std::string_view entry{value};
      const std::size_t separator = entry.find("::");
      if (separator == std::string_view::npos) {
        if (test.name == entry) return true;
      } else if (normalized == entry.substr(0, separator) &&
                 test.name == entry.substr(separator + 2)) {
        return true;
      }
    }
    return false;
  };
  const auto matches = [&](const test_case& test) {
    std::string normalized{test.file};
    for (char& value : normalized) {
      if (value == '\\') value = '/';
    }
    return (file.empty() || normalized.find(file) != std::string::npos) &&
           (name.empty() || test.name == name);
  };
  std::size_t selected = 0;
  for (const auto& test : tests()) {
    if (matches(test) && !is_excluded(test)) selected++;
  }
  if (selected == 0) {
    std::fputs("No tests matched\n", stderr);
    return 2;
  }
  if (list) {
    for (const auto& test : tests()) {
      if (!matches(test) || is_excluded(test)) continue;
      std::printf("%.*s:%d - %.*s\n", static_cast<int>(test.file.size()),
                  test.file.data(), test.line,
                  static_cast<int>(test.name.size()), test.name.data());
    }
    return 0;
  }
  common::console_logger logger{
      "unit_tests", common::console_logger_options{.show_function = false,
                                                   .show_line = false}};
  static constexpr std::string_view kText =
      "     _       _\n"
      "  __| | ___ | |__   __ _\n"
      " / _` |/ _ \\| '_ \\ / _` |\n"
      "| (_| | (_) | |_) | (_| |\n"
      " \\__,_|\\___/|_.__/ \\__,_|\n";
  std::fwrite(kText.data(), 1, kText.size(), stdout);
  std::fputc('\n', stdout);
  std::fflush(stdout);
  logger.info() << "running " << selected << " unit tests";
  std::size_t failed_tests = 0;
  std::size_t skipped_tests = 0;
  for (const auto& test : tests()) {
    if (!matches(test)) continue;
    if (is_excluded(test)) {
      skipped_tests++;
      std::printf("skipped %.*s:%d - %.*s\n",
                  static_cast<int>(test.file.size()), test.file.data(),
                  test.line, static_cast<int>(test.name.size()),
                  test.name.data());
      std::fflush(stdout);
      continue;
    }
    context().clear();
    const std::size_t failures_before = failures();
    std::printf("running %.*s:%d - %.*s\n",
                static_cast<int>(test.file.size()), test.file.data(), test.line,
                static_cast<int>(test.name.size()), test.name.data());
    std::fflush(stdout);
    try {
      test.test();
    } catch (const std::exception& error) {
      failures()++;
      std::fprintf(stderr, "Exception: %s\n", error.what());
    } catch (...) {
      failures()++;
      std::fputs("Unknown exception\n", stderr);
    }
    if (failures() != failures_before) {
      failed_tests++;
      logger.error() << test.file << ':' << test.line << " - " << test.name
                     << common::console_log_color::kRed << " failed";
    } else {
      logger.info() << test.file << ':' << test.line << " - " << test.name
                    << common::console_log_color::kGreen << " passed";
    }
  }
  if (skipped_tests != 0) {
    logger.info() << skipped_tests
                  << " test(s) skipped by explicit exclusions";
  }
  if (failed_tests != 0) {
    logger.error() << failed_tests << " test(s) failed";
    return 1;
  }
  return 0;
}
}  // namespace martianlabs::doba::tests::unit

// +===========================================================================+
// | [>] main                                                  ( entry-point ) |
// +===========================================================================+
int main(int argc, char** argv) {
  return martianlabs::doba::tests::unit::test_helper::run(argc, argv);
}
