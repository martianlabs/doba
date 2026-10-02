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

namespace martianlabs::doba::tests::integration {
namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] test_case                                                  ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct represents a single test case in the integration test         |
// | framework. It contains the file name, line number, test name, and a       |
// | function pointer to the test function. The test function is expected      |
// | to return a boolean value indicating whether the test passed or failed.   |
// | The struct is used to store and manage test cases in the integration      |
// | test framework.                                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct test_case {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::string_view file;
  int line;
  std::string_view name;
  test_helper::test_t test;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] relative_test_path                                       ( function ) |
// +---------------------------------------------------------------------------+
// | This function takes a file path as input and returns a relative path by   |
// | removing the leading directories up to and including                      |
// | the "tests/integration/"                                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::string_view relative_test_path(std::string_view file) {
  constexpr std::string_view windows_marker = "\\tests\\integration\\";
  constexpr std::string_view unix_marker = "/tests/integration/";
  std::size_t pos = file.rfind(windows_marker);
  if (pos == std::string_view::npos) pos = file.rfind(unix_marker);
  if (pos != std::string_view::npos) file.remove_prefix(pos + 1);
  return file;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tests                                                    ( function ) |
// +---------------------------------------------------------------------------+
// | This function returns a reference to a static vector of                   |
// | test_case objects. It is used to store and manage the test cases in the   |
// | integration test framework. The vector is initialized only once and       |
// | persists for the lifetime of the program, allowing test cases to be added |
// | and accessed throughout the execution of the tests.                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::vector<test_case>& tests() {
  static std::vector<test_case> value;
  return value;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] failures                                                 ( function ) |
// +---------------------------------------------------------------------------+
// | This function returns a reference to a static size_t variable that keeps  |
// | track of the number of test failures. It is used to count the number of   |
// | failed test cases in the integration test framework. The variable is      |
// | initialized to zero and persists for the lifetime of the program,         |
// | allowing the failure count to be updated and accessed throughout the      |
// | execution of the tests.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::size_t& failures() {
  static std::size_t value = 0;
  return value;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context                                                  ( function ) |
// +---------------------------------------------------------------------------+
// | This function returns a reference to a static std::string variable that   |
// | keeps track of the current test context. It is used to store and access   |
// | the context information for the test cases in the integration test        |
// | framework. The variable is initialized only once and persists for the     |
// | lifetime of the program, allowing the context to be updated and accessed  |
// | throughout the execution of the tests.                                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::string& context() {
  static std::string value;
  return value;
}
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] add                                                      ( function ) |
// +---------------------------------------------------------------------------+
// | This function adds a new test case to the integration test framework.     |
// | It takes the file name, line number, test name, and a function pointer to |
// | the test function as parameters. The function creates a new test_case     |
// | object with the provided information and appends it to the static vector  |
// | of test cases. The function returns true to indicate that the test case   |
// | was successfully added.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool test_helper::add(std::string_view file, int line, std::string_view name,
                      test_t test) {
  tests().push_back({relative_test_path(file), line, name, test});
  return true;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] expect                                                   ( function ) |
// +---------------------------------------------------------------------------+
// | This function checks whether a given condition is true or false.          |
// | It takes a boolean condition, a string_view expression representing the   |
// | assertion, the file name, and the line number as parameters. If the       |
// | condition is true, it returns true. If the condition is false, it         |
// | increments the failure count, prints an error message to stderr with the  |
// | file name, line number, and failed assertion expression, and              |
// | returns false. The function also prints the current test context if       |
// | it is not empty.                                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
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

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] set_context                                              ( function ) |
// +---------------------------------------------------------------------------+
// | This function sets the current test context to the provided string_view   |
// | value. It assigns the value to a static std::string variable that         |
// | keeps track of the current test context. This allows the context          |
// | information to be updated and accessed throughout the execution of the    |
// | tests, providing additional information for debugging and                 |
// | reporting test results.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void test_helper::set_context(std::string_view value) {
  context().assign(value);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] run                                                      ( function ) |
// +---------------------------------------------------------------------------+
// | This function runs the tests with the provided command-line arguments.    |
// | It parses the arguments to determine which tests to run, list, or exclude |
// | and then executes the matching tests, providing detailed output for each  |
// | test case.                                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int test_helper::run(int argc, char** argv) {
  bool list = false;
  std::string file;
  std::string_view name;
  std::string_view exclude;
  for (int i = 1; i < argc; i++) {
    const std::string_view argument{argv[i]};
    if (argument == "--list") {
      list = true;
    } else if ((argument == "--file" || argument == "--name" ||
                argument == "--exclude") &&
               i + 1 < argc) {
      const std::string_view value{argv[++i]};
      if (argument == "--file") file = value;
      if (argument == "--name") name = value;
      if (argument == "--exclude") exclude = value;
    } else {
      std::fputs("Invalid test arguments\n", stderr);
      return 2;
    }
  }
  for (char& value : file) {
    if (value == '\\') value = '/';
  }
  const auto matches = [&](const test_case& test) {
    std::string normalized{test.file};
    for (char& value : normalized) {
      if (value == '\\') value = '/';
    }
    return (file.empty() || normalized.find(file) != std::string::npos) &&
           (name.empty() || test.name == name) &&
           (exclude.empty() || test.name != exclude);
  };
  std::size_t selected = 0;
  for (const auto& test : tests()) {
    if (matches(test)) selected++;
  }
  if (selected == 0) {
    std::fputs("No tests matched\n", stderr);
    return 2;
  }
  if (list) {
    for (const auto& test : tests()) {
      if (!matches(test)) continue;
      std::printf("%.*s:%d - %.*s\n", static_cast<int>(test.file.size()),
                  test.file.data(), test.line,
                  static_cast<int>(test.name.size()), test.name.data());
    }
    return 0;
  }
  common::console_logger logger{
      "integration_tests", common::console_logger_options{
                               .show_function = false, .show_line = false}};
  static constexpr std::string_view kText =
      "     _       _\n"
      "  __| | ___ | |__   __ _\n"
      " / _` |/ _ \\| '_ \\ / _` |\n"
      "| (_| | (_) | |_) | (_| |\n"
      " \\__,_|\\___/|_.__/ \\__,_|\n";
  std::fwrite(kText.data(), 1, kText.size(), stdout);
  std::fputc('\n', stdout);
  std::fflush(stdout);
  logger.info() << "running " << selected << " integration tests";
  std::size_t failed_tests = 0;
  for (const auto& test : tests()) {
    if (!matches(test)) continue;
    context().clear();
    const std::size_t failures_before = failures();
    std::printf("running %.*s:%d - %.*s\n", static_cast<int>(test.file.size()),
                test.file.data(), test.line, static_cast<int>(test.name.size()),
                test.name.data());
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
  if (failed_tests != 0) {
    logger.error() << failed_tests << " test(s) failed";
    return 1;
  }
  return 0;
}
}  // namespace martianlabs::doba::tests::integration

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                   ( entry-pint ) |
// +---------------------------------------------------------------------------+
// | This is the entry point of the integration test framework.                |
// | It calls the run function of the test_helper class, passing the           |
// | command-line arguments to it. The return value of the run function is     |
// | returned as the exit code of the program, indicating the                  |
// | success or failure of the tests.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  return martianlabs::doba::tests::integration::test_helper::run(argc, argv);
}
