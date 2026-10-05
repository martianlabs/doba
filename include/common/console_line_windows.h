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

#ifndef martianlabs_doba_common_console_line_windows_h
#define martianlabs_doba_common_console_line_windows_h

  // +=========================================================================+
// | [>] line                                                        ( class ) |
  // +=========================================================================+
  class line {
   public:
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                             ( public ) |
    // +=======================================================================+
    // +=======================================================================+
    // | [>] CONSTRUCTORs/DESTRUCTORs                               ( public ) |
    // +=======================================================================+
    line(const line&) = delete;
    line(line&& in) noexcept
        : logger_(std::exchange(in.logger_, nullptr)),
          level_(in.level_),
          source_(in.source_),
          message_(std::move(in.message_)),
          colored_message_(std::move(in.colored_message_)),
          color_(in.color_) {}
    ~line() {
      if (logger_) {
        logger_->write(level_, message_, colored_message_, source_);
      }
    }
    // +=======================================================================+
    // | [>] OPERATORs                                              ( public ) |
    // +=======================================================================+
    line& operator=(const line&) = delete;
    line& operator=(line&&) noexcept = delete;
    line& operator<<(std::string_view value) {
      message_.append(value);
      colored_message_.append(value);
      return *this;
    }
    line& operator<<(const char* value) {
      return *this << std::string_view{value};
    }
    line& operator<<(char value) {
      message_ += value;
      colored_message_ += value;
      return *this;
    }
    line& operator<<(bool value) {
      return *this << std::string_view{value ? "true" : "false"};
    }
    template <typename Ty>
      requires(std::is_arithmetic_v<Ty> &&
               !std::is_same_v<std::remove_cv_t<Ty>, char> &&
               !std::is_same_v<std::remove_cv_t<Ty>, bool>)
    line& operator<<(Ty value) {
      char text[128]{};
      auto result = std::to_chars(text, text + sizeof(text), value);
      if (result.ec == std::errc{}) {
        *this << std::string_view{text,
                                  static_cast<std::size_t>(result.ptr - text)};
      }
      return *this;
    }
    line& operator<<(console_log_color color) {
      if (color_ != color) {
        color_ = color;
        colored_message_.append(console_logger::color_code(color));
      }
      return *this;
    }

   private:
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                            ( private ) |
    // +=======================================================================+
    // +=======================================================================+
    // | [>] FRIENDs                                               ( private ) |
    // +=======================================================================+
    friend class console_logger;
    // +=======================================================================+
    // | [>] CONSTRUCTORs                                          ( private ) |
    // +=======================================================================+
    line(console_logger& logger, console_log_level level,
         std::source_location source)
        : logger_(&logger), level_(level), source_(source) {}
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                            ( private ) |
    // +=======================================================================+
    console_logger* logger_;
    console_log_level level_;
    std::source_location source_;
    std::string message_;
    std::string colored_message_;
    console_log_color color_{console_log_color::kDefault};
  };

#endif
