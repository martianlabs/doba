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

#ifndef martianlabs_doba_tests_helpers_tool_json_h
#define martianlabs_doba_tests_helpers_tool_json_h

#include "tool_support.h"

#include <cmath>
#include <iomanip>

namespace martianlabs::doba::tests::tools {

// +---------------------------------------------------------------------------+
// | [>] json                                                        ( class ) |
// +---------------------------------------------------------------------------+
// Private tooling value. Reject ambiguous objects, malformed numbers, invalid
// Unicode escapes, excessive nesting and trailing input instead of guessing.
class json {
 public:
  // +-------------------------------------------------------------------------+
  // | [>] PUBLIC INTERFACE                                         ( public ) |
  // +-------------------------------------------------------------------------+
  using array = std::vector<json>;
  using object = std::map<std::string, json>;

 private:
  // +-------------------------------------------------------------------------+
  // | [>] IMPLEMENTATION                                          ( private ) |
  // +-------------------------------------------------------------------------+
  enum class kind { empty, boolean, number, text, sequence, mapping };
  kind kind_{kind::empty};
  bool boolean_{};
  double number_{};
  std::string text_;
  array sequence_;
  object mapping_;

  // +-------------------------------------------------------------------------+
  // | [>] parser                                                    ( class ) |
  // +-------------------------------------------------------------------------+
  class parser {
    // +-----------------------------------------------------------------------+
    // | [>] ATTRIBUTEs                                            ( private ) |
    // +-----------------------------------------------------------------------+
    std::string_view input_;
    size_t position_{};

    // +-----------------------------------------------------------------------+
    // | [>] take                                                  ( private ) |
    // +-----------------------------------------------------------------------+
    char take() {
      require(position_ < input_.size(), "Truncated JSON");
      return input_[position_++];
    }
    // +-----------------------------------------------------------------------+
    // | [>] whitespace                                            ( private ) |
    // +-----------------------------------------------------------------------+
    void whitespace() {
      while (position_ < input_.size() &&
        std::string_view(" \n\r\t").find(input_[position_]) !=
          std::string_view::npos) ++position_;
    }
    // +-----------------------------------------------------------------------+
    // | [>] hex                                                   ( private ) |
    // +-----------------------------------------------------------------------+
    unsigned hex() {
      unsigned result = 0;
      for (int index = 0; index < 4; ++index) {
        char digit = take();
        unsigned number = digit >= '0' && digit <= '9' ? digit - '0' :
          digit >= 'a' && digit <= 'f' ? digit - 'a' + 10 :
          digit >= 'A' && digit <= 'F' ? digit - 'A' + 10 : 16;
        require(number < 16, "Invalid JSON Unicode escape");
        result = result * 16 + number;
      }
      return result;
    }
    // +-----------------------------------------------------------------------+
    // | [>] utf8                                                  ( private ) |
    // +-----------------------------------------------------------------------+
    static void utf8(std::string& text, unsigned code) {
      if (code < 128) text += static_cast<char>(code);
      else {
        unsigned count = code < 2048 ? 2 : code < 65536 ? 3 : 4;
        text += static_cast<char>((count == 2 ? 0xc0 :
          count == 3 ? 0xe0 : 0xf0) | (code >> (6 * (count - 1))));
        for (unsigned index = count - 1; index > 0; --index) {
          text += static_cast<char>(0x80 | ((code >> (6 * (index - 1))) & 63));
        }
      }
    }
    // +-----------------------------------------------------------------------+
    // | [>] string                                                ( private ) |
    // +-----------------------------------------------------------------------+
    std::string string() {
      require(take() == '"', "Expected JSON string");
      std::string result;
      for (;;) {
        auto character = static_cast<unsigned char>(take());
        if (character == '"') return result;
        require(character >= 32, "Control character in JSON string");
        if (character != '\\') {
          if (character >= 128) {
            unsigned count = character >= 0xc2 && character <= 0xdf ? 1 :
              character >= 0xe0 && character <= 0xef ? 2 :
              character >= 0xf0 && character <= 0xf4 ? 3 : 0;
            require(count != 0, "Invalid JSON UTF-8 lead byte");
            unsigned code = character & (count == 1 ? 31 : count == 2 ? 15 : 7);
            std::string encoded(1, static_cast<char>(character));
            for (unsigned index = 0; index < count; ++index) {
              auto next = static_cast<unsigned char>(take());
              require((next & 0xc0) == 0x80, "Invalid JSON UTF-8 continuation");
              encoded += static_cast<char>(next);
              code = code * 64 + (next & 63);
            }
            require(code >= (count == 1 ? 128u : count == 2 ? 2048u : 65536u) &&
              code <= 0x10ffff && (code < 0xd800 || code > 0xdfff),
              "Invalid JSON UTF-8 code point");
            result += encoded;
            continue;
          }
          result += static_cast<char>(character);
          continue;
        }
        switch (take()) {
          case '"': result += '"'; break;
          case '\\': result += '\\'; break;
          case '/': result += '/'; break;
          case 'b': result += '\b'; break;
          case 'f': result += '\f'; break;
          case 'n': result += '\n'; break;
          case 'r': result += '\r'; break;
          case 't': result += '\t'; break;
          case 'u': {
            unsigned code = hex();
            if (code >= 0xd800 && code <= 0xdbff) {
              require(take() == '\\' && take() == 'u',
                "Missing JSON low surrogate");
              unsigned low = hex();
              require(low >= 0xdc00 && low <= 0xdfff,
                "Invalid JSON low surrogate");
              code = 65536 + (code - 0xd800) * 1024 + low - 0xdc00;
            } else {
              require(code < 0xdc00 || code > 0xdfff,
                "Unpaired JSON low surrogate");
            }
            utf8(result, code);
            break;
          }
          default: throw std::runtime_error("Invalid JSON escape");
        }
      }
    }
    // +-----------------------------------------------------------------------+
    // | [>] value                                                 ( private ) |
    // +-----------------------------------------------------------------------+
    json value(unsigned depth) {
      require(depth < 128, "JSON nesting limit exceeded");
      whitespace();
      require(position_ < input_.size(), "Missing JSON value");
      char first = input_[position_];
      if (first == '"') return string();
      if (first == '{' || first == '[') {
        ++position_;
        json result = first == '{' ? json(object{}) : json(array{});
        whitespace();
        char end = first == '{' ? '}' : ']';
        if (position_ < input_.size() && input_[position_] == end) {
          ++position_;
          return result;
        }
        for (;;) {
          whitespace();
          if (first == '{') {
            auto key = string();
            whitespace();
            require(take() == ':', "Expected JSON colon");
            auto inserted = result.members().emplace(key, value(depth + 1));
            require(inserted.second, "Duplicate JSON object key");
          } else result.items().push_back(value(depth + 1));
          whitespace();
          char separator = take();
          if (separator == end) return result;
          require(separator == ',', "Expected JSON separator");
        }
      }
      for (auto literal : {"true", "false", "null"}) {
        std::string_view token(literal);
        if (input_.substr(position_, token.size()) == token) {
          position_ += token.size();
          if (token == "null") return nullptr;
          return token == "true";
        }
      }
      size_t begin = position_;
      if (first == '-') ++position_;
      auto digit = [&] {
        return position_ < input_.size() && input_[position_] >= '0' &&
          input_[position_] <= '9';
      };
      require(digit(), "Invalid JSON number");
      if (input_[position_] == '0') ++position_;
      else while (digit()) ++position_;
      if (position_ < input_.size() && input_[position_] == '.') {
        ++position_;
        require(digit(), "Missing JSON fraction");
        while (digit()) ++position_;
      }
      if (position_ < input_.size() &&
          (input_[position_] == 'e' || input_[position_] == 'E')) {
        ++position_;
        if (position_ < input_.size() &&
            (input_[position_] == '+' || input_[position_] == '-')) ++position_;
        require(digit(), "Missing JSON exponent");
        while (digit()) ++position_;
      }
      double number = std::stod(std::string(input_.substr(
        begin, position_ - begin)));
      require(std::isfinite(number), "Nonfinite JSON number");
      return number;
    }

  public:
    // +-----------------------------------------------------------------------+
    // | [>] CONSTRUCTORs                                           ( public ) |
    // +-----------------------------------------------------------------------+
    explicit parser(std::string_view input) : input_(input) {}
    // +-----------------------------------------------------------------------+
    // | [>] parse                                                  ( public ) |
    // +-----------------------------------------------------------------------+
    json parse() {
      auto result = value(0);
      whitespace();
      require(position_ == input_.size(), "Trailing JSON input");
      return result;
    }
  };

 public:
  // +-------------------------------------------------------------------------+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  json() = default;
  json(std::nullptr_t) {}
  json(bool value) : kind_(kind::boolean), boolean_(value) {}
  json(double value) : kind_(kind::number), number_(value) {}
  json(int value) : json(static_cast<double>(value)) {}
  json(size_t value) : json(static_cast<double>(value)) {}
  json(const char* value) : json(std::string(value)) {}
  json(std::string value) : kind_(kind::text), text_(std::move(value)) {}
  json(array value) : kind_(kind::sequence), sequence_(std::move(value)) {}
  json(object value) : kind_(kind::mapping), mapping_(std::move(value)) {}
  // +-------------------------------------------------------------------------+
  // | [>] null                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool null() const { return kind_ == kind::empty; }
  // +-------------------------------------------------------------------------+
  // | [>] numeric                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  bool numeric() const { return kind_ == kind::number; }
  // +-------------------------------------------------------------------------+
  // | [>] number                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  double number() const {
    require(numeric(), "Expected JSON number");
    return number_;
  }
  // +-------------------------------------------------------------------------+
  // | [>] boolean                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  bool boolean() const {
    require(kind_ == kind::boolean, "Expected JSON boolean");
    return boolean_;
  }
  // +-------------------------------------------------------------------------+
  // | [>] string                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  const std::string& string() const {
    require(kind_ == kind::text, "Expected JSON string");
    return text_;
  }
  // +-------------------------------------------------------------------------+
  // | [>] items                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  const array& items() const {
    require(kind_ == kind::sequence, "Expected JSON array");
    return sequence_;
  }
  // +-------------------------------------------------------------------------+
  // | [>] items                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  array& items() {
    require(kind_ == kind::sequence, "Expected JSON array");
    return sequence_;
  }
  // +-------------------------------------------------------------------------+
  // | [>] members                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  const object& members() const {
    require(kind_ == kind::mapping, "Expected JSON object");
    return mapping_;
  }
  // +-------------------------------------------------------------------------+
  // | [>] members                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  object& members() {
    require(kind_ == kind::mapping, "Expected JSON object");
    return mapping_;
  }
  // +-------------------------------------------------------------------------+
  // | [>] contains                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  bool contains(const std::string& key) const {
    return members().contains(key);
  }
  json& operator[](const std::string& key) {
    if (null()) kind_ = kind::mapping;
    return members()[key];
  }
  const json& operator[](const std::string& key) const {
    return members().at(key);
  }
  json& operator[](size_t index) { return items().at(index); }
  const json& operator[](size_t index) const { return items().at(index); }
  bool operator==(const json&) const = default;
  // +-------------------------------------------------------------------------+
  // | [>] parse                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  static json parse(std::string_view input) { return parser(input).parse(); }
  // +-------------------------------------------------------------------------+
  // | [>] load                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  static json load(const fs::path& path) { return parse(read(path)); }
  // +-------------------------------------------------------------------------+
  // | [>] quote                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  static std::string quote(std::string_view text) {
    std::ostringstream output;
    output << '"';
    for (unsigned char character : text) {
      switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
          if (character < 32) {
            output << "\\u00" << std::hex << std::setw(2) <<
              std::setfill('0') << static_cast<unsigned>(character);
          } else output << static_cast<char>(character);
      }
    }
    return output.str() + '"';
  }
  // +-------------------------------------------------------------------------+
  // | [>] dump                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  std::string dump() const {
    if (null()) return "null";
    if (kind_ == kind::boolean) return boolean_ ? "true" : "false";
    if (numeric()) {
      require(std::isfinite(number_), "Cannot serialize nonfinite JSON");
      std::ostringstream output;
      output << std::setprecision(17) << number_;
      return output.str();
    }
    if (kind_ == kind::text) return quote(text_);
    std::string output;
    if (kind_ == kind::sequence) {
      for (const auto& item : sequence_) {
        if (!output.empty()) output += ',';
        output += item.dump();
      }
      return '[' + output + ']';
    }
    for (const auto& [key, item] : members()) {
      if (!output.empty()) output += ',';
      output += quote(key) + ':' + item.dump();
    }
    return '{' + output + '}';
  }
};

inline void write_json(const fs::path& path, const json& value) {
  write(path, value.dump() + '\n');
}

} // namespace martianlabs::doba::tests::tools

#endif
