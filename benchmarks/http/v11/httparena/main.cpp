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

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <zlib.h>

#include "common/filesystem.h"
#include "common/reader.h"
#include "common/task.h"
#include "http_server.h"
#include "https_server.h"
#include "protocol/http/common/helpers.h"
#include "transport/server/tls.h"

using martianlabs::doba::http_server;
using namespace martianlabs::doba::protocol::http::v11;

namespace {
namespace common = martianlabs::doba::common;
using martianlabs::doba::protocol::http::helpers;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] encoding_weights                                         ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool encoding_weights(std::string_view value, std::array<int, 4>& quality) {
  constexpr std::array<std::string_view, 4> names = {
      "br", "gzip", "identity", "*"};
  return helpers::for_each_list_element(
      value, [&quality, &names](std::string_view element) {
        helpers::ows_trim(element);
        const auto coding = helpers::consume_token(element);
        if (coding.empty()) return false;
        auto weight = element.substr(coding.size());
        helpers::ows_trim(weight);
        int q = 1000;
        if (!weight.empty()) {
          if (!helpers::consume_weight(weight)) return false;
          const auto number = weight.substr(weight.find('=') + 1);
          q = (number.front() - '0') * 1000;
          int scale = 100;
          for (std::size_t digit = 2; digit < number.size(); ++digit) {
            q += (number[digit] - '0') * scale;
            scale /= 10;
          }
        }
        for (std::size_t j = 0; j < names.size(); ++j) {
          if (helpers::iequals(coding, names[j])) {
            // Conflicting duplicates must not override a q=0 rejection.
            quality[j] = quality[j] < 0 ? q : std::min(quality[j], q);
            break;
          }
        }
        return true;
      });
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] static_content_type                                      ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::string_view static_content_type(std::string_view path) {
  const auto dot = path.rfind('.');
  if (dot == path.npos) return "application/octet-stream";
  const auto extension = path.substr(dot);
  constexpr std::pair<std::string_view, std::string_view> types[] = {
      {".html", "text/html"}, {".htm", "text/html"},
      {".css", "text/css"}, {".js", "text/javascript"},
      {".json", "application/json"}, {".txt", "text/plain"},
      {".svg", "image/svg+xml"}, {".png", "image/png"},
      {".jpg", "image/jpeg"}, {".jpeg", "image/jpeg"},
      {".gif", "image/gif"}, {".webp", "image/webp"},
      {".ico", "image/vnd.microsoft.icon"}, {".pdf", "application/pdf"},
      {".wasm", "application/wasm"}, {".woff2", "font/woff2"},
  };
  for (const auto& type : types) {
    if (helpers::iequals(extension, type.first)) return type.second;
  }
  return "application/octet-stream";
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] serve_static                                             ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void serve_static(const request& req, response& result,
                  const std::filesystem::path& root) {
  constexpr std::string_view prefix = "/static/";
  const auto path = req.get_absolute_path();
  if (!path.starts_with(prefix)) {
    result.not_found_404();
    return;
  }
  const auto relative = path.substr(prefix.size());
  if (relative.empty() || relative.back() == '/') {
    result.not_found_404();
    return;
  }
  std::error_code error;
  // Validate the original path before appending a representation suffix.
  if (!common::detail::filesystem_relative_path(relative, error)) {
    result.forbidden_403();
    return;
  }

  constexpr std::array<std::string_view, 4> encodings = {
      "br", "gzip", "identity", "*"};
  std::array<int, 4> quality = {-1, -1, -1, -1};
  bool if_match = false;
  bool match_star = false;
  bool none_star = false;
  for (std::size_t i = 0; i < req.get_headers_length(); ++i) {
    const auto header = req.get_header(i);
    auto value = header.second;
    helpers::ows_trim(value);
    if (helpers::iequals(header.first, "Accept-Encoding")) {
      if (!encoding_weights(value, quality)) {
        result.bad_request_400();
        return;
      }
    } else if (helpers::iequals(header.first, "If-Match")) {
      if_match = true;
      match_star = match_star || value == "*";
    } else if (helpers::iequals(header.first, "If-None-Match")) {
      none_star = none_star || value == "*";
    }
  }
  for (std::size_t i = 0; i < 2; ++i) {
    if (quality[i] < 0) quality[i] = std::max(0, quality[3]);
  }
  // RFC 9110 S12.5.3: implicit identity is a fallback unless *;q=0 excludes it.
  if (quality[2] < 0) quality[2] = quality[3] == 0 ? 0 : 1;
  std::array<std::size_t, 3> order = {0, 1, 2};
  std::sort(order.begin(), order.end(), [&quality](auto left, auto right) {
    return quality[left] == quality[right] ? left < right
                                         : quality[left] > quality[right];
  });

  common::filesystem_file file;
  std::size_t selected = encodings.size();
  constexpr std::array<std::string_view, 3> suffixes = {".br", ".gz", ""};
  for (const auto index : order) {
    if (quality[index] == 0) continue;
    std::string candidate(relative);
    candidate += suffixes[index];
    // Open directly; do not stat/read the original or reopen the chosen file.
    if (file.open(root, candidate, error)) {
      selected = index;
      break;
    }
    if (error == std::errc::no_such_file_or_directory ||
        error == std::errc::not_a_directory ||
        error == std::errc::is_a_directory) continue;
    if (error == std::errc::permission_denied ||
        error == std::errc::operation_not_permitted ||
        error == std::errc::too_many_symbolic_link_levels) {
      result.forbidden_403();
      return;
    }
    result.internal_server_error_500();
    return;
  }
  if (selected == encodings.size()) {
    if (quality[2] != 0) {
      result.not_found_404();
      return;
    }
    result.not_acceptable_406();
    result.set_header("Vary", "Accept-Encoding");
    return;
  }
  // Preserve Doba's existing precondition order (RFC 9110 S13.2.2).
  if (if_match && !match_star) {
    result.precondition_failed_412();
    return;
  }
  if (none_star) {
    result.not_modified_304();
    result.clear_body();
    result.set_header("Vary", "Accept-Encoding");
    return;
  }
  result.ok_200();
  result.set_header("Content-Type", static_content_type(relative));
  result.set_header("Vary", "Accept-Encoding");
  if (selected != 2) result.set_header("Content-Encoding", encodings[selected]);
  const auto length = file.size();
  if (req.get_method() == "HEAD") {
    result.set_header("Content-Length", std::to_string(length));
  } else {
    result.set_body(common::reader(std::move(file)), length);
  }
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parse_integer                                            ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool parse_integer(std::string_view source, std::int64_t& value) {
  if (source.empty()) return false;
  const char* first = source.data();
  const char* last = first + source.size();
  const auto [end, error] = std::from_chars(first, last, value);
  return error == std::errc() && end == last;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] read_query_sum                                           ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool read_query_sum(const request& req, std::int64_t& value) {
  const auto a = req.get_query_parameter("a");
  const auto b = req.get_query_parameter("b");
  std::int64_t a_value = 0;
  std::int64_t b_value = 0;
  if (!a || !b || !parse_integer(a->second, a_value) ||
      !parse_integer(b->second, b_value)) {
    return false;
  }
  value = a_value + b_value;
  return true;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] read_body_integer                                        ( function ) |
// +---------------------------------------------------------------------------+
// | Reads decoded bytes for Content-Length and chunked bodies.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool read_body_integer(const request& req, std::int64_t& value) {
  if (!req.has_body_reader()) return false;
  std::array<std::byte, 64> buffer{};
  std::size_t used = 0;
  for (;;) {
    const auto state =
        req.get_body_reader()->read(std::span(buffer).subspan(used));
    if (state.has_error) return false;
    used += state.produced;
    if (state.complete) break;
    if (used == buffer.size()) return false;
  }
  return parse_integer(
      std::string_view(reinterpret_cast<const char*>(buffer.data()), used),
      value);
}
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Runs the HttpArena benchmark routes on TCP and TLS.                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  const char* dataset_path = std::getenv("DATASET_PATH");
  std::ifstream dataset_file(dataset_path ? dataset_path
                                          : "/data/dataset.json");
  if (!dataset_file) {
    std::cerr << "unable to open dataset\n";
    return 1;
  }
  const std::string dataset_text(std::istreambuf_iterator<char>{dataset_file},
                                 std::istreambuf_iterator<char>{});
  rapidjson::Document dataset;
  dataset.Parse(dataset_text.c_str());
  if (dataset.HasParseError() || !dataset.IsArray()) {
    std::cerr << "invalid dataset\n";
    return 1;
  }
  policies http_configuration;
  std::error_code static_error;
  const auto static_root =
      common::filesystem_root("/data/static", static_error);
  if (static_error) {
    std::cerr << "unable to open static root\n";
    return 1;
  }
  http_server<> http_server({.ip = "0.0.0.0", .port = "8080"},
                     http_configuration);
  http_server.add_route(
      "GET", "/delay/:ms",
      [](const request&, response& res, std::uint32_t ms)
          -> martianlabs::doba::common::task<void> {
        if (ms != 0) {
          co_await martianlabs::doba::common::sleep_for(
              std::chrono::milliseconds(ms));
        }
        res.ok_200();
        res.add_header("Content-Type", "text/plain")
            .set_body(std::to_string(ms));
      });
  http_server.add_route(
      "GET", "/pipeline",
      [](const request&, response& res) {
        res.ok_200();
        res.add_header("Content-Type", "text/plain").set_body("ok");
      });
  // Parse every baseline value; HttpArena randomizes them to detect shortcuts.
  http_server.add_route(
      "GET", "/baseline11",
      [](const request& req, response& res) {
        res.ok_200();
        std::int64_t value = 0;
        if (!read_query_sum(req, value)) {
          res.bad_request_400();
          res.set_body("invalid request");
          return;
        }
        res.add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value));
      });
  http_server.add_route(
      "POST", "/baseline11",
      [](const request& req, response& res) {
        res.ok_200();
        std::int64_t value = 0;
        std::int64_t body_value = 0;
        if (!read_query_sum(req, value) ||
            !read_body_integer(req, body_value)) {
          res.bad_request_400();
          res.set_body("invalid request");
          return;
        }
        res.add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value + body_value));
      });
  auto json_handler =
      [&dataset](const request& req, response& res,
                 std::uint64_t requested_count) {
        res.ok_200();
        std::int64_t multiplier = 1;
        const auto m = req.get_query_parameter("m");
        if (m && !parse_integer(m->second, multiplier)) {
          res.bad_request_400();
          res.set_body("invalid multiplier");
          return;
        }
        const auto count = std::min<std::uint64_t>(requested_count,
                                                   dataset.Size());
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        writer.StartObject();
        writer.Key("items");
        writer.StartArray();
        for (std::uint64_t i = 0; i < count; ++i) {
          const auto& item = dataset[static_cast<rapidjson::SizeType>(i)];
          writer.StartObject();
          for (auto field = item.MemberBegin(); field != item.MemberEnd();
               ++field) {
            field->name.Accept(writer);
            field->value.Accept(writer);
          }
          writer.Key("total");
          writer.Int64(item["price"].GetInt64() *
                       item["quantity"].GetInt64() * multiplier);
          writer.EndObject();
        }
        writer.EndArray();
        writer.Key("count");
        writer.Uint64(count);
        writer.EndObject();

        std::string_view body(buffer.GetString(), buffer.GetSize());
        res.add_header("Content-Type", "application/json");
        bool gzip = false;
        if (req.exist_header("Accept-Encoding")) {
          martianlabs::doba::protocol::http::helpers::for_each_list_element(
              req.get_header("Accept-Encoding").second,
              [&gzip](std::string_view encoding) {
                const auto separator = encoding.find(';');
                if (martianlabs::doba::protocol::http::helpers::iequals(
                        encoding.substr(0, separator), "gzip")) {
                  gzip = true;
                  if (separator != std::string_view::npos) {
                    auto weight = encoding.substr(separator + 1);
                    martianlabs::doba::protocol::http::helpers::ows_ltrim(
                        weight);
                    if (weight.starts_with("q=")) {
                      const auto value = weight.substr(2);
                      gzip = value.find_first_of("123456789") !=
                             std::string_view::npos;
                    }
                  }
                }
                return true;
              });
        }
        if (gzip) {
          z_stream stream{};
          if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                           MAX_WBITS + 16, MAX_MEM_LEVEL,
                           Z_DEFAULT_STRATEGY) != Z_OK) {
            res.internal_server_error_500();
            return;
          }
          std::string compressed(deflateBound(&stream, body.size()), '\0');
          stream.next_in = reinterpret_cast<Bytef*>(
              const_cast<char*>(body.data()));
          stream.avail_in = static_cast<uInt>(body.size());
          stream.next_out = reinterpret_cast<Bytef*>(compressed.data());
          stream.avail_out = static_cast<uInt>(compressed.size());
          const int result = deflate(&stream, Z_FINISH);
          deflateEnd(&stream);
          if (result != Z_STREAM_END) {
            res.internal_server_error_500();
            return;
          }
          compressed.resize(stream.total_out);
          res.add_header("Content-Encoding", "gzip");
          res.set_body(compressed);
        } else {
          res.set_body(body);
        }
      };
  http_server.add_route("GET", "/json/:count", json_handler);
  martianlabs::doba::transport::server::tls_policies tls_configuration;
  tls_configuration.ip = "0.0.0.0";
  tls_configuration.port = "8081";
  tls_configuration.certificate_file = "/certs/server.crt";
  tls_configuration.private_key_file = "/certs/server.key";
  martianlabs::doba::https_server<> tls_server(tls_configuration,
                                               http_configuration);
  tls_server.add_route("GET", "/json/:count", json_handler);
  const auto static_handler =
      [static_root](const request& req, response& res) {
        serve_static(req, res, static_root);
      };
  tls_server.add_route("GET", "/static/*", static_handler);
  tls_server.add_route("HEAD", "/static/*", static_handler);
  tls_server.add_route(
      "POST", "/echo",
      [](const request& req, response& res) {
        res.ok_200();
        res.add_header("Content-Type", "application/octet-stream")
            .set_body(req.get_body_reader());
      });
  http_server.start();
  tls_server.start();
  // Docker owns process shutdown; wait after the server starts.
  std::promise<void> shutdown;
  shutdown.get_future().wait();
  return 0;
}
