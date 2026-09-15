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

#include <chrono>
#include <initializer_list>
#include <string>

#include "../../helpers/test_helper_process.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::tests::test_helper_result;
using martianlabs::doba::tests::unit::test_helper;

// +===========================================================================+
// | [>] run_probe                                                  ( method ) |
// +===========================================================================+
test_helper_result run_probe(
    std::initializer_list<const char*> arguments,
    std::chrono::milliseconds timeout = std::chrono::seconds(3)) {
  auto result = martianlabs::doba::tests::run_test_helper(arguments, timeout);
  test_helper::set_context(result.output);
  return result;
}
}  // namespace

// +===========================================================================+
// | [>] listing does not execute tests                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("listing does not execute tests") {
const auto result = run_probe({"--list"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 0);
  DOBA_EXPECT(result.output.find("probe passes") != std::string::npos);
  DOBA_EXPECT(result.output.find("probe body") == std::string::npos);
}

// +===========================================================================+
// | [>] file and name filters select one test                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("file and name filters select one test") {
const auto result = run_probe(
      {"--file", "test_helper_probe.cpp", "--name", "probe passes"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 0);
  DOBA_EXPECT(result.output.find("probe body") != std::string::npos);
  DOBA_EXPECT(result.output.find("probe continued") == std::string::npos);
}

// +===========================================================================+
// | [>] unmatched filter reports an error                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("unmatched filter reports an error") {
const auto result = run_probe({"--name", "nonexistent"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 2);
  DOBA_EXPECT(result.output.find("No tests matched") != std::string::npos);
}

// +===========================================================================+
// | [>] assertion failure reports expression and location       ( test-case ) |
// +===========================================================================+
DOBA_TEST("assertion failure reports expression and location") {
const auto result = run_probe({"--name", "probe assertion fails"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 1);
  DOBA_EXPECT(result.output.find("2 + 2 == 5") != std::string::npos);
  const std::string marker{"test_helper_probe.cpp:"};
  const auto location = result.output.find(marker);
  DOBA_EXPECT(location != std::string::npos);
  const auto line = location + marker.size();
  DOBA_EXPECT(line < result.output.size());
  DOBA_EXPECT(result.output[line] >= '0' && result.output[line] <= '9');
}

// +===========================================================================+
// | [>] standard exception reports its message                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("standard exception reports its message") {
const auto result = run_probe({"--name", "probe standard exception"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 1);
  DOBA_EXPECT(result.output.find("probe standard error") != std::string::npos);
}

// +===========================================================================+
// | [>] unknown exception reports an error                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("unknown exception reports an error") {
const auto result = run_probe({"--name", "probe unknown exception"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 1);
  DOBA_EXPECT(result.output.find("Unknown exception") != std::string::npos);
}

// +===========================================================================+
// | [>] execution continues after failed tests                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("execution continues after failed tests") {
const auto result = run_probe(
      {"--file", "test_helper_probe.cpp", "--exclude", "probe blocks"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 1);
  DOBA_EXPECT(result.output.find("probe continued") != std::string::npos);
}

// +===========================================================================+
// | [>] unknown argument reports an error                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("unknown argument reports an error") {
const auto result = run_probe({"--unknown"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 2);
  DOBA_EXPECT(result.output.find("Invalid test arguments") !=
              std::string::npos);
}

// +===========================================================================+
// | [>] assertion failure reports row context                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("assertion failure reports row context") {
const auto result = run_probe({"--name", "probe row context"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 1);
  DOBA_EXPECT(result.output.find("case: row 7: input=invalid") !=
              std::string::npos);
}

// +===========================================================================+
// | [>] blocked test is identified before timeout               ( test-case ) |
// +===========================================================================+
DOBA_TEST("blocked test is identified before timeout") {
const auto result = run_probe({"--name", "probe blocks"},
                                std::chrono::seconds(1));
  DOBA_EXPECT(result.timed_out);
  const auto name = result.output.find(" - probe blocks");
  DOBA_EXPECT(name != std::string::npos);
  const auto line = result.output.rfind('\n', name);
  const auto start = line == std::string::npos ? 0 : line + 1;
  DOBA_EXPECT_EQUAL(result.output.compare(start, 8, "running "), 0);
}

// +===========================================================================+
// | [>] listing applies repeated exclusions                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("listing applies repeated exclusions") {
const auto result = run_probe(
      {"--list", "--exclude", "probe assertion fails",
       "--exclude", "probe row context"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 0);
  DOBA_EXPECT(result.output.find("probe passes") != std::string::npos);
  DOBA_EXPECT(result.output.find("probe assertion fails") ==
              std::string::npos);
  DOBA_EXPECT(result.output.find("probe row context") == std::string::npos);
}

// +===========================================================================+
// | [>] exclusions accept names and normalized file paths       ( test-case ) |
// +===========================================================================+
DOBA_TEST("exclusions accept names and normalized file paths") {
const auto result = run_probe(
      {"--exclude",
       "tests/unit/helpers/test_helper_probe.cpp::probe assertion fails",
       "--exclude",
       "tests\\unit\\helpers\\test_helper_probe.cpp::probe standard exception",
       "--exclude", "probe unknown exception",
       "--exclude", "probe row context",
       "--exclude", "probe blocks"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 0);
  DOBA_EXPECT(result.output.find("5 test(s) skipped by explicit exclusions") !=
              std::string::npos);
  DOBA_EXPECT(result.output.find("probe body") != std::string::npos);
  DOBA_EXPECT(result.output.find("probe continued") != std::string::npos);
  DOBA_EXPECT(result.output.find("assertion failed:") == std::string::npos);
  DOBA_EXPECT(result.output.find("Exception:") == std::string::npos);
  DOBA_EXPECT(result.output.find("Unknown exception") == std::string::npos);
}

// +===========================================================================+
// | [>] exclusion does not match another file                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("exclusion does not match another file") {
const auto result = run_probe(
      {"--name", "probe assertion fails", "--exclude",
       "tests/integration/helpers/test_helper_probe.cpp::"
       "probe assertion fails"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 1);
  DOBA_EXPECT(result.output.find("2 + 2 == 5") != std::string::npos);
}

// +===========================================================================+
// | [>] exclusion preserves unrelated failures                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("exclusion preserves unrelated failures") {
const auto result = run_probe(
      {"--name", "probe unknown exception", "--exclude",
       "tests/unit/helpers/test_helper_probe.cpp::probe assertion fails"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 1);
  DOBA_EXPECT(result.output.find("Unknown exception") != std::string::npos);
}

// +===========================================================================+
// | [>] excluding the only selected test reports an error       ( test-case ) |
// +===========================================================================+
DOBA_TEST("excluding the only selected test reports an error") {
const auto result = run_probe(
      {"--name", "probe assertion fails", "--exclude",
       "tests/unit/helpers/test_helper_probe.cpp::probe assertion fails"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 2);
  DOBA_EXPECT(result.output.find("No tests matched") != std::string::npos);
}

// +===========================================================================+
// | [>] exclusion without a value reports an error              ( test-case ) |
// +===========================================================================+
DOBA_TEST("exclusion without a value reports an error") {
const auto result = run_probe({"--exclude"});
  DOBA_EXPECT(!result.timed_out);
  DOBA_EXPECT_EQUAL(result.exit_code, 2);
  DOBA_EXPECT(result.output.find("Invalid test arguments") !=
              std::string::npos);
}
