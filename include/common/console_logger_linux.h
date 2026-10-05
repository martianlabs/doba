//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#ifndef martianlabs_doba_common_console_logger_linux_h
#define martianlabs_doba_common_console_logger_linux_h

#include <charconv>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] console_logger [linux]                                      ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class console_logger {
 public:
#include "common/console_line_linux.h"
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  explicit console_logger(std::string_view name,
                          console_logger_options options = {})
      : name_(name), options_(options) {}
  console_logger(const console_logger&) = delete;
  console_logger(console_logger&&) noexcept = delete;
  ~console_logger() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  console_logger& operator=(const console_logger&) = delete;
  console_logger& operator=(console_logger&&) noexcept = delete;
  // +=========================================================================+
  // | [>] debug                                                    ( public ) |
  // +=========================================================================+
  line debug(std::source_location source = std::source_location::current()) {
    return line{*this, console_log_level::kDebug, source};
  }
  void debug(std::string_view message,
             std::source_location source = std::source_location::current()) {
    write(console_log_level::kDebug, message, message, source);
  }
  // +=========================================================================+
  // | [>] info                                                     ( public ) |
  // +=========================================================================+
  line info(std::source_location source = std::source_location::current()) {
    return line{*this, console_log_level::kInfo, source};
  }
  void info(std::string_view message,
            std::source_location source = std::source_location::current()) {
    write(console_log_level::kInfo, message, message, source);
  }
  // +=========================================================================+
  // | [>] warning                                                  ( public ) |
  // +=========================================================================+
  line warning(std::source_location source = std::source_location::current()) {
    return line{*this, console_log_level::kWarning, source};
  }
  void warning(std::string_view message,
               std::source_location source = std::source_location::current()) {
    write(console_log_level::kWarning, message, message, source);
  }
  // +=========================================================================+
  // | [>] error                                                    ( public ) |
  // +=========================================================================+
  line error(std::source_location source = std::source_location::current()) {
    return line{*this, console_log_level::kError, source};
  }
  void error(std::string_view message,
             std::source_location source = std::source_location::current()) {
    write(console_log_level::kError, message, message, source);
  }
  // +=========================================================================+
  // | [>] critical                                                 ( public ) |
  // +=========================================================================+
  line critical(std::source_location source = std::source_location::current()) {
    return line{*this, console_log_level::kCritical, source};
  }
  void critical(std::string_view message,
                std::source_location source = std::source_location::current()) {
    write(console_log_level::kCritical, message, message, source);
  }

 private:
  // +=========================================================================+
  // | [>] append_field                                            ( private ) |
  // +=========================================================================+
  static void append_field(std::string& output, std::string_view value) {
    if (!output.empty()) output += ' ';
    output.append(value);
  }
  // +=========================================================================+
  // | [>] append_timestamp                                        ( private ) |
  // +=========================================================================+
  static void append_timestamp(std::string& output) {
    std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local{};
    if (!localtime_r(&now, &local)) return;
    char text[20]{};
    if (std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S", &local) == 0) {
      return;
    }
    append_field(output, text);
  }
  // +=========================================================================+
  // | [>] output_mutex                                            ( private ) |
  // +=========================================================================+
  static std::mutex& output_mutex() {
    static std::mutex mutex;
    return mutex;
  }
  // +=========================================================================+
  // | [>] color_code                                              ( private ) |
  // +=========================================================================+
  static std::string_view color_code(console_log_color color) {
    switch (color) {
      case console_log_color::kDefault:
        return "\x1b[0m";
      case console_log_color::kBlack:
        return "\x1b[30m";
      case console_log_color::kRed:
        return "\x1b[31m";
      case console_log_color::kGreen:
        return "\x1b[32m";
      case console_log_color::kYellow:
        return "\x1b[33m";
      case console_log_color::kBlue:
        return "\x1b[34m";
      case console_log_color::kMagenta:
        return "\x1b[35m";
      case console_log_color::kCyan:
        return "\x1b[36m";
      case console_log_color::kWhite:
        return "\x1b[37m";
    }
    return {};
  }
  // +=========================================================================+
  // | [>] level_color                                             ( private ) |
  // +=========================================================================+
  static std::string_view level_color(console_log_level level) {
    switch (level) {
      case console_log_level::kDebug:
        return color_code(console_log_color::kGreen);
      case console_log_level::kInfo:
        return color_code(console_log_color::kCyan);
      case console_log_level::kWarning:
        return color_code(console_log_color::kYellow);
      case console_log_level::kError:
        return color_code(console_log_color::kRed);
      case console_log_level::kCritical:
        return "\x1b[97;41m";
    }
    return {};
  }
  // +=========================================================================+
  // | [>] level_label                                             ( private ) |
  // +=========================================================================+
  static std::string_view level_label(console_log_level level) {
    switch (level) {
      case console_log_level::kDebug:
        return "[DBG]";
      case console_log_level::kInfo:
        return "[INF]";
      case console_log_level::kWarning:
        return "[WRN]";
      case console_log_level::kError:
        return "[ERR]";
      case console_log_level::kCritical:
        return "[CRT]";
    }
    return {};
  }
  // +=========================================================================+
  // | [>] write                                                   ( private ) |
  // +=========================================================================+
  void write(console_log_level level, std::string_view message,
             std::string_view colored_message,
             const std::source_location& source) {
    std::string timestamp;
    if (options_.show_timestamp) append_timestamp(timestamp);
    std::string output{level_label(level)};
    if (options_.show_name) output += "[" + name_ + "]";
    if (options_.show_timestamp) append_field(output, timestamp);
    if (options_.show_function) append_field(output, source.function_name());
    std::string line = std::to_string(source.line());
    if (options_.show_line) append_field(output, line);
    if (!message.empty()) append_field(output, message);

    std::lock_guard<std::mutex> lock(output_mutex());
    if (isatty(fileno(stdout))) {
      std::fputs(level_color(level).data(), stdout);
      std::fputs(level_label(level).data(), stdout);
      std::fputs(color_code(console_log_color::kDefault).data(), stdout);
      if (options_.show_name) {
        std::fputc('[', stdout);
        std::fwrite(name_.data(), 1, name_.size(), stdout);
        std::fputc(']', stdout);
      }
      if (options_.show_timestamp) {
        std::fputc(' ', stdout);
        std::fwrite(timestamp.data(), 1, timestamp.size(), stdout);
      }
      if (options_.show_function) {
        std::fputc(' ', stdout);
        std::fputs(source.function_name(), stdout);
      }
      if (options_.show_line) {
        std::fputc(' ', stdout);
        std::fwrite(line.data(), 1, line.size(), stdout);
      }
      if (!message.empty()) {
        std::fputc(' ', stdout);
        std::fwrite(colored_message.data(), 1, colored_message.size(), stdout);
        std::fputs(color_code(console_log_color::kDefault).data(), stdout);
      }
      std::fputc('\n', stdout);
    } else {
      std::fwrite(output.data(), 1, output.size(), stdout);
      std::fputc('\n', stdout);
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string name_;
  console_logger_options options_;
};
}  // namespace martianlabs::doba::common

#endif
