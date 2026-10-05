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


#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "common/console_logger.h"
#include "output_capture.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::tests::unit::output_capture;
using martianlabs::doba::common::console_log_color;
using martianlabs::doba::common::console_logger;

}  // namespace

// +===========================================================================+
// | [>] logger emits the label for every severity               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("logger emits the label for every severity") {
  output_capture capture;
  console_logger value{"unit", {false, false, false, false}};
  value.debug("debug");
  value.info("info");
  value.warning("warning");
  value.error("error");
  value.critical("critical");
  const auto output = capture.output();
  DOBA_EXPECT_EQUAL(output,
                    "[DBG] debug\n[INF] info\n[WRN] warning\n"
                    "[ERR] error\n[CRT] critical\n");
}

// +===========================================================================+
// | [>] logger owns its name and uses the supplied location     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("logger owns its name and uses the supplied location") {
  output_capture capture;
  std::string name = "original";
  console_logger value{name, {true, false, true, true}};
  name.assign("changed");
  const auto source = std::source_location::current();
  value.info("message", source);
  const std::string expected = "[INF][original] " +
                               std::string(source.function_name()) + " " +
                               std::to_string(source.line()) + " message\n";
  DOBA_EXPECT_EQUAL(capture.output(), expected);
}

// +===========================================================================+
// | [>] logger preserves empty and binary messages              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("logger preserves empty and binary messages") {
  output_capture capture;
  console_logger value{"unit", {false, false, false, false}};
  value.info("");
  value.info(std::string_view("a\0b\xff", 4));
  std::string expected = "[INF]\n[INF] ";
  expected.append("a\0b\xff", 4);
  expected += '\n';
  DOBA_EXPECT_EQUAL(capture.output(), expected);
}

// +===========================================================================+
// | [>] logger streams scalar limits without losing bytes       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("logger streams scalar limits without losing bytes") {
  output_capture capture;
  console_logger value{"unit", {false, false, false, false}};
  {
    auto line = value.warning();
    line << std::string_view("a\0b", 3) << ' ' << true << ' ' << false << ' '
         << std::numeric_limits<int64_t>::min() << ' '
         << std::numeric_limits<uint64_t>::max() << ' ' << 1.25;
  }
  std::string expected = "[WRN] ";
  expected.append("a\0b", 3);
  expected += " true false -9223372036854775808 18446744073709551615 1.25\n";
  DOBA_EXPECT_EQUAL(capture.output(), expected);
}

// +===========================================================================+
// | [>] moved logger lines commit the message exactly once      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("moved logger lines commit the message exactly once") {
  output_capture capture;
  console_logger value{"unit", {false, false, false, false}};
  {
    auto source = value.info();
    source << "first";
    auto destination = std::move(source);
    destination << " second";
  }
  DOBA_EXPECT_EQUAL(capture.output(), "[INF] first second\n");
}

// +===========================================================================+
// | [>] redirected logger output omits terminal color codes     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("redirected logger output omits terminal color codes") {
  output_capture capture;
  console_logger value{"unit", {false, false, false, false}};
  {
    auto line = value.error();
    for (console_log_color color :
         {console_log_color::kDefault, console_log_color::kBlack,
          console_log_color::kRed, console_log_color::kGreen,
          console_log_color::kYellow, console_log_color::kBlue,
          console_log_color::kMagenta, console_log_color::kCyan,
          console_log_color::kWhite}) {
      line << color << color << 'x';
    }
  }
  DOBA_EXPECT_EQUAL(capture.output(), "[ERR] xxxxxxxxx\n");
}

// +===========================================================================+
// | [>] logger options enable each metadata field               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("logger options enable each metadata field") {
  const martianlabs::doba::common::console_logger_options value;
  DOBA_EXPECT(value.show_name);
  DOBA_EXPECT(value.show_timestamp);
  DOBA_EXPECT(value.show_function);
  DOBA_EXPECT(value.show_line);
}
