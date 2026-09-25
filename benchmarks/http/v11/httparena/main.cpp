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
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <zlib.h>

#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::protocol::http::v11;

namespace {
bool parse_integer(std::string_view source, std::int64_t& value) {
  if (source.empty()) return false;
  const char* first = source.data();
  const char* last = first + source.size();
  const auto [end, error] = std::from_chars(first, last, value);
  return error == std::errc() && end == last;
}

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

// The body reader exposes decoded bytes for Content-Length and chunked bodies.
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

int main(int argc, char* argv[]) {
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
  server http_server({.ip = "0.0.0.0", .port = "8080"});
  // Parse every baseline value; HttpArena randomizes them to detect shortcuts.
  http_server.add_route(
      "GET", "/baseline11",
      [](const request& req) {
        response res = response::ok_200();
        std::int64_t value = 0;
        if (!read_query_sum(req, value)) {
          res = response::bad_request_400();
          res.set_body("invalid request");
          return res;
        }
        res.add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value));
        return res;
      });
  http_server.add_route(
      "POST", "/baseline11",
      [](const request& req) {
        response res = response::ok_200();
        std::int64_t value = 0;
        std::int64_t body_value = 0;
        if (!read_query_sum(req, value) ||
            !read_body_integer(req, body_value)) {
          res = response::bad_request_400();
          res.set_body("invalid request");
          return res;
        }
        res.add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value + body_value));
        return res;
      });
  http_server.add_route(
      "GET", "/json/:count",
      [&dataset](const request& req, std::uint64_t requested_count) {
        response res = response::ok_200();
        std::int64_t multiplier = 1;
        const auto m = req.get_query_parameter("m");
        if (m && !parse_integer(m->second, multiplier)) {
          res = response::bad_request_400();
          res.set_body("invalid multiplier");
          return res;
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
            return response::internal_server_error_500();
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
            return response::internal_server_error_500();
          }
          compressed.resize(stream.total_out);
          res.add_header("Content-Encoding", "gzip");
          res.set_body(compressed);
        } else {
          res.set_body(body);
        }
        return res;
      });
  // Keep this handler minimal so the profile isolates pipelining overhead.
  http_server.add_route(
      "GET", "/pipeline",
      [](const request& req) {
        response res = response::ok_200();
        res.add_header("Content-Type", "text/plain").set_body("ok");
        return res;
      });
  http_server.start();
  // Docker owns process shutdown; wait after the server starts.
  std::promise<void> shutdown;
  shutdown.get_future().wait();
  return 0;
}
