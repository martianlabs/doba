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

#ifndef martianlabs_doba_protocol_http_v11_static_file_server_h
#define martianlabs_doba_protocol_http_v11_static_file_server_h

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "common/filesystem.h"
#include "common/reader.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"

namespace martianlabs::doba::protocol::http::v11 {
// +---------------------------------------------------------------------------+
// | [>] static_file_server                                          ( class ) |
// +---------------------------------------------------------------------------+

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] static_file_server                                          ( class ) |
// +---------------------------------------------------------------------------+
// | Server implementation.                                                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class static_file_server {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+

  static_file_server(std::string_view prefix,
                     const std::filesystem::path& root)
      : prefix_(prefix) {
    if (prefix_.empty() || prefix_.front() != '/' ||
        prefix_.find_first_of("*:?#\\") != prefix_.npos) {
      throw std::invalid_argument("Invalid static file URL prefix");
    }
    while (prefix_.size() > 1 && prefix_.back() == '/') prefix_.pop_back();
    if (prefix_.size() > 1) {
      std::error_code error;
      if (!common::detail::filesystem_relative_path(
              std::string_view(prefix_).substr(1), error)) {
        throw std::invalid_argument("Invalid static file URL prefix");
      }
      prefix_ += '/';
    }
    std::error_code error;
    root_ = common::filesystem_root(root, error);
    if (error) {
      throw std::filesystem::filesystem_error(
          "Unable to open static file root", root, error);
    }
  }
  // +=========================================================================+
  // | [>] register_routes                                          ( public ) |
  // +=========================================================================+

  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", prefix_ + "*", &static_file_server::serve);
    routes.add("HEAD", prefix_ + "*", &static_file_server::serve);
  }

 private:
  // +=========================================================================+
  // | [>] serve                                                   ( private ) |
  // +=========================================================================+

  response serve(const request& req) const {
    const auto path = req.get_absolute_path();
    if (!path.starts_with(prefix_)) return response::not_found_404();
    const auto relative = path.substr(prefix_.size());
    if (relative.empty() || relative.back() == '/') {
      return response::not_found_404();
    }
    common::filesystem_file file;
    std::error_code error;
    if (!file.open(root_, relative, error)) {
      if (error == std::errc::no_such_file_or_directory ||
          error == std::errc::not_a_directory ||
          error == std::errc::is_a_directory) {
        return response::not_found_404();
      }
      if (error == std::errc::permission_denied ||
          error == std::errc::operation_not_permitted ||
          error == std::errc::too_many_symbolic_link_levels) {
        return response::forbidden_403();
      }
      return response::internal_server_error_500();
    }
    // RFC 9110 S13.2.2: evaluate If-Match before If-None-Match.
    bool if_match = false;
    bool match_star = false;
    bool none_star = false;
    for (std::size_t i = 0; i < req.get_headers_length(); i++) {
      const auto header = req.get_header(i);
      auto value = header.second;
      helpers::ows_trim(value);
      if (helpers::iequals(header.first, "If-Match")) {
        if_match = true;
        match_star = match_star || value == "*";
      } else if (helpers::iequals(header.first, "If-None-Match")) {
        none_star = none_star || value == "*";
      }
    }
    if (if_match && !match_star) return response::precondition_failed_412();
    if (none_star) {
      auto result = response::not_modified_304();
      result.clear_body();
      return result;
    }
    auto result = response::ok_200();
    result.set_header("Content-Type", content_type(relative));
    const auto length = file.size();
    if (req.get_method() == "HEAD") {
      result.set_header("Content-Length", std::to_string(length));
    } else {
      result.set_body(common::reader(std::move(file)), length);
    }
    return result;
  }
  // +=========================================================================+
  // | [>] content_type                                            ( private ) |
  // +=========================================================================+

  static std::string_view content_type(std::string_view path) {
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
        {".wasm", "application/wasm"},
    };
    for (const auto& type : types) {
      if (helpers::iequals(extension, type.first)) return type.second;
    }
    return "application/octet-stream";
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+

  std::string prefix_;
  std::filesystem::path root_;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
