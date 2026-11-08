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

#ifndef martianlabs_doba_protocol_http_v11_static_file_server_h
#define martianlabs_doba_protocol_http_v11_static_file_server_h

#include <array>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "common/filesystem.h"
#include "common/cache.h"
#include "common/reader.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "protocol/http/v11/static_file_helpers.h"

namespace martianlabs::doba::protocol::http::v11 {
// Freeze snapshots before caching. Charge bytes.size(), sizeof(this type) and
// 66 ETag characters; don't include allocator, key or control-block overhead.
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] static_file_representation                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct static_file_representation {
  std::string bytes;
  common::filesystem_metadata metadata;
  std::string etag;
  // Format this zero-terminated HTTP date once, at load time.
  std::array<char, 40> last_modified{};
};
using static_file_cache =
    common::cache<std::string, static_file_representation>;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] static_file_policies                                       ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct static_file_policies {
  // No cache means direct reads. Share one cache to share its byte budget.
  std::shared_ptr<static_file_cache> cache{};
  std::size_t max_cached_file_size = 1024 * 1024;
  std::string cache_control = "no-cache";
  bool deny_hidden = true;
  bool ranges = true;
  bool precompressed = false;
  bool redirect_directories = false;
  std::string index_file{};
  std::string spa_fallback{};
  std::vector<std::string> excluded{};
  std::vector<std::pair<std::string, std::string>> mime_types{};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] static_file_server                                          ( class ) |
// +---------------------------------------------------------------------------+
// | Serve files with optional caching and HTTP validators.                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class static_file_server {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  static_file_server(std::string_view prefix, const std::filesystem::path& root,
                     static_file_policies policies = {})
      : prefix_(prefix), policies_(std::move(policies)) {
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
      throw std::filesystem::filesystem_error("Unable to open static file root",
                                              root, error);
    }
    const auto valid_relative = [](const std::string& value) {
      std::error_code ec;
      return value.empty() ||
             common::detail::filesystem_relative_path(value, ec);
    };
    if (!valid_relative(policies_.index_file) ||
        policies_.index_file.find('/') != policies_.index_file.npos ||
        !valid_relative(policies_.spa_fallback))
      throw std::invalid_argument("Invalid static file index or fallback");
    if (policies_.cache_control.find_first_of("\r\n") != std::string::npos)
      throw std::invalid_argument("Invalid static file cache policy");
    for (const auto& [extension, mime] : policies_.mime_types) {
      if (extension.empty() || extension.front() != '.' || mime.empty() ||
          mime.find_first_of("\r\n") != mime.npos)
        throw std::invalid_argument("Invalid static file MIME mapping");
    }
    const auto root_key = root_.generic_u8string();
    key_prefix_.assign(root_key.begin(), root_key.end());
    key_prefix_ += '\0';
  }
  // +=========================================================================+
  // | [>] register_routes                                          ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", prefix_ + "*", &static_file_server::serve);
    routes.add("HEAD", prefix_ + "*", &static_file_server::serve);
    if (policies_.redirect_directories && prefix_.size() > 1) {
      routes.add("GET", prefix_.substr(0, prefix_.size() - 1),
                 &static_file_server::serve);
      routes.add("HEAD", prefix_.substr(0, prefix_.size() - 1),
                 &static_file_server::serve);
    }
  }

  // Pass a decoded relative path, without a leading slash. Invalidate future
  // lookups and loads; active replies keep their snapshots.
  // +-------------------------------------------------------------------------+
  // | [>] invalidate                                               ( public ) |
  // +-------------------------------------------------------------------------+
  void invalidate(std::string_view relative) const {
    if (policies_.cache)
      for (std::string_view suffix : {"", ".br", ".gz"})
        policies_.cache->erase(key_prefix_ + std::string(relative) +
                               std::string(suffix));
  }
  // +-------------------------------------------------------------------------+
  // | [>] clear_cache                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void clear_cache() const {
    if (policies_.cache) policies_.cache->clear();
  }

 private:
  // +=========================================================================+
  // | [>] serve                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void serve(const request& req, response& result) const {
    const auto path = req.get_absolute_path();
    if (policies_.redirect_directories && prefix_.size() > 1 &&
        path == std::string_view(prefix_).substr(0, prefix_.size() - 1)) {
      redirect(req, result, std::string(path) + '/');
      return;
    }
    if (!path.starts_with(prefix_)) {
      result.not_found_404();
      return;
    }
    std::string relative(path.substr(prefix_.size()));
    const bool directory_path = relative.empty() || relative.back() == '/';
    if (directory_path) {
      if (policies_.index_file.empty()) {
        result.not_found_404();
        return;
      }
      relative += policies_.index_file;
    }
    std::error_code error;
    if (!common::detail::filesystem_relative_path(relative, error) ||
        denied(relative)) {
      result.forbidden_403();
      return;
    }
    try { serve_file(req, result, relative, path);
    } catch (const std::filesystem::filesystem_error& failure) {
      file_error(result, failure.code());
    }
  }

  // +-------------------------------------------------------------------------+
  // | [>] denied                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  bool denied(std::string_view relative) const {
    for (const auto& excluded : policies_.excluded) {
      if (relative == excluded || (relative.starts_with(excluded) &&
                                   relative.size() > excluded.size() &&
                                   relative[excluded.size()] == '/'))
        return true;
    }
    if (policies_.deny_hidden) {
      for (std::size_t pos = 0; pos < relative.size(); ++pos)
        if ((pos == 0 || relative[pos - 1] == '/') && relative[pos] == '.')
          return true;
    }
    return false;
  }

  // +-------------------------------------------------------------------------+
  // | [>] file_error                                              ( private ) |
  // +-------------------------------------------------------------------------+
  static void file_error(response& result, const std::error_code& error) {
    if (error == std::errc::no_such_file_or_directory ||
        error == std::errc::not_a_directory ||
        error == std::errc::is_a_directory)
      result.not_found_404();
    else if (error == std::errc::permission_denied ||
             error == std::errc::operation_not_permitted ||
             error == std::errc::too_many_symbolic_link_levels)
      result.forbidden_403();
    else result.internal_server_error_500();
  }

  // +-------------------------------------------------------------------------+
  // | [>] field                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  static std::string field(const request& req, std::string_view name) {
    std::string result;
    for (std::size_t i = 0; i < req.get_headers_length(); ++i) {
      const auto header = req.get_header(i);
      if (helpers::iequals(header.first, name)) {
        if (!result.empty()) result += ',';
        result += header.second;
      }
    }
    return result;
  }
  // +-------------------------------------------------------------------------+
  // | [>] has_field                                               ( private ) |
  // +-------------------------------------------------------------------------+
  static bool has_field(const request& req, std::string_view name) {
    for (std::size_t i = 0; i < req.get_headers_length(); ++i)
      if (helpers::iequals(req.get_header(i).first, name)) return true;
    return false;
  }

  // +-------------------------------------------------------------------------+
  // | [>] redirect                                                ( private ) |
  // +-------------------------------------------------------------------------+
  static void redirect(const request& req, response& result,
                       std::string_view path) {
    // Escape the decoded path again so a filename can't change the URL's
    // meaning.
    std::string location;
    constexpr char hex[] = "0123456789ABCDEF";
    for (unsigned char c : path) {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '/' || c == '-' || c == '_' ||
          c == '.' || c == '~')
        location += char(c);
      else {
        location += '%';
        location += hex[c >> 4];
        location += hex[c & 15];
      }
    }
    // Keep query values escaped exactly as received.
    for (std::size_t i = 0; i < req.get_query_parameters_length(); ++i) {
      const auto query = req.get_query_parameter(i);
      location += (i ? '&' : '?');
      location += query.first;
      location += '=';
      location += query.second;
    }
    result.moved_permanently_301();
    result.set_header("Location", location);
  }

  // Choose an encoding, respecting identity;q=0 and wildcard exclusions.
  // +-------------------------------------------------------------------------+
  // | [>] quality                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  static int quality(std::string_view fields, std::string_view coding) {
    if (fields.empty()) return coding == "identity" ? 1000 : 0;
    int specific = -1, wildcard = -1;
    while (!fields.empty()) {
      const auto comma = fields.find(',');
      auto item = fields.substr(0, comma);
      helpers::ows_trim(item);
      const auto semicolon = item.find(';');
      auto name = item.substr(0, semicolon);
      helpers::ows_trim(name);
      int q = 1000;
      if (semicolon != item.npos) {
        auto weight = item.substr(semicolon + 1);
        helpers::ows_trim(weight);
        if (weight.starts_with("q=") || weight.starts_with("Q=")) {
          weight.remove_prefix(2);
          q = 0;
          if (!weight.empty() && weight.front() == '1') q = 1000;
          else if (weight.starts_with("0.")) {
            int multiplier = 100;
            for (char c : weight.substr(2)) {
              if (c < '0' || c > '9') {
                q = 0;
                break;
              }
              q += (c - '0') * multiplier;
              multiplier /= 10;
            }
          }
        }
      }
      if (helpers::iequals(name, coding)) specific = q;
      if (name == "*") wildcard = q;
      if (comma == fields.npos) break;
      fields.remove_prefix(comma + 1);
    }
    if (specific >= 0) return specific;
    if (coding == "identity") return wildcard == 0 ? 0 : 1000;
    return wildcard >= 0 ? wildcard : 0;
  }

  // +=========================================================================+
  // | [>] open_file                                               ( private ) |
  // +-------------------------------------------------------------------------+
  bool open_file(const std::string& selected, common::filesystem_file& file,
                 std::shared_ptr<const static_file_representation>& cached,
                 std::optional<common::filesystem_metadata>& opened,
                 std::error_code& error) const {
    error.clear();
    opened.reset();
    cached.reset();
    if (!policies_.cache) {
      if (!file.open(root_, selected, error)) return false;
      opened = file.metadata(error);
      return opened.has_value();
    }
    const auto key = key_prefix_ + selected;
    // Check attributes before opening for reads. A missing variant needs only
    // this confined query, not a read handle or failed cache load.
    opened = common::filesystem_file::inspect(root_, selected, error);
    if (policies_.cache) {
      cached = policies_.cache->find(key);
      if (cached) {
        if (opened && cached->metadata == *opened) return true;
        policies_.cache->erase(key, cached);
        cached.reset();
      }
    }
    if (!opened) return false;
    if (!file.open(root_, selected, error)) return false;
    opened = file.metadata(error);
    return opened.has_value();
  }

  // +-------------------------------------------------------------------------+
  // | [>] read_representation                                     ( private ) |
  // +-------------------------------------------------------------------------+
  static std::shared_ptr<const static_file_representation> read_representation(
      common::filesystem_file& file,
      const common::filesystem_metadata& expected, std::error_code& error) {
    auto value = std::make_shared<static_file_representation>();
    value->bytes.resize(file.size());
    std::size_t position = 0;
    while (position < value->bytes.size()) {
      const auto count = file.read(
          std::as_writable_bytes(
              std::span(value->bytes.data(), value->bytes.size()))
              .subspan(position));
      if (!count)
        throw std::filesystem::filesystem_error(
            "Static file read failed",
            std::make_error_code(std::errc::io_error));
      position += count;
    }
    auto after = file.metadata(error);
    if (!after || expected != *after) return {};
    value->metadata = *after;
    value->etag = detail::static_file_etag(value->bytes);
    const auto date =
        detail::static_file_date(value->metadata.modified_seconds);
    std::copy(date.begin(), date.end(), value->last_modified.begin());
    return value;
  }
  // +-------------------------------------------------------------------------+
  // | [>] serve_file                                              ( private ) |
  // +-------------------------------------------------------------------------+
  void serve_file(const request& req, response& result, std::string relative,
                  std::string_view url_path) const {
    struct variant {
      std::string_view suffix, coding;
      int q;
    };
    const auto accept_encoding = field(req, "Accept-Encoding");
    const auto brotli_quality =
        policies_.precompressed ? quality(accept_encoding, "br") : 0;
    const auto gzip_quality =
        policies_.precompressed ? quality(accept_encoding, "gzip") : 0;
    const auto identity_quality = quality(accept_encoding, "identity");
    std::array<variant, 3> variants{{
        {".br", "br", brotli_quality},
        {".gz", "gzip", gzip_quality},
        {"", "identity", identity_quality}}};
    // Sort three choices without a heap allocation.
    if (variants[1].q > variants[0].q) std::swap(variants[0], variants[1]);
    if (variants[2].q > variants[1].q) std::swap(variants[1], variants[2]);
    if (variants[1].q > variants[0].q) std::swap(variants[0], variants[1]);
    common::filesystem_file file;
    std::error_code error;
    std::shared_ptr<const static_file_representation> cached;
    std::optional<common::filesystem_metadata> opened;
    std::string selected;
    std::string_view coding;
    bool acceptable = false;
    for (const auto& variant : variants) {
      if (!variant.q) continue;
      acceptable = true;
      selected = relative + std::string(variant.suffix);
      // Revalidate every hit without a content-read handle. TTL only controls
      // retention; changed attributes retire the entry.
      if (open_file(selected, file, cached, opened, error)) {
        coding = variant.coding;
        break;
      }
      if (error != std::errc::no_such_file_or_directory &&
          error != std::errc::not_a_directory) {
        if (variant.suffix.empty() && error == std::errc::is_a_directory) break;
        file_error(result, error);
        return;
      }
    }
    if (!acceptable) {
      result.not_acceptable_406();
      if (policies_.precompressed) result.set_header("Vary", "Accept-Encoding");
      return;
    }
    if (!cached && !file.is_open()) {
      if (!quality(accept_encoding, "identity")) {
        common::filesystem_file original;
        if (original.open(root_, relative, error)) {
          result.not_acceptable_406();
          if (policies_.precompressed)
            result.set_header("Vary", "Accept-Encoding");
          return;
        }
      }
      if (error == std::errc::is_a_directory &&
          policies_.redirect_directories) {
        redirect(req, result, std::string(url_path) + '/');
        return;
      }
      if ((error == std::errc::no_such_file_or_directory ||
           error == std::errc::not_a_directory) &&
          !policies_.spa_fallback.empty() && !denied(policies_.spa_fallback) &&
          quality(accept_encoding, "identity")) {
        relative = policies_.spa_fallback;
        selected = relative;
        if (!open_file(selected, file, cached, opened, error)) {
          file_error(result, error);
          return;
        }
        coding = "identity";
      } else {
        file_error(result, error);
        return;
      }
    }
    common::filesystem_metadata metadata;
    if (!cached) {
      metadata = *opened;
      if (policies_.cache && metadata.size == file.size() &&
          file.size() <= policies_.max_cached_file_size &&
          file.size() <= std::numeric_limits<std::size_t>::max() -
                             sizeof(static_file_representation) - 66) {
        bool loaded = false;
        cached = policies_.cache->try_get_or_load(
            key_prefix_ + selected,
            file.size() + sizeof(static_file_representation) + 66,
            [&loaded, &file, &opened, &error] {
              loaded = true;
              return read_representation(file, *opened, error);
            });
        // Another load may have filled this key meanwhile. Never attach cached
        // bytes from a different observed file.
        if (cached) {
          if (cached->metadata != *opened) {
            policies_.cache->erase(key_prefix_ + selected, cached);
            cached.reset();
          }
        }
        if (error) {
          file_error(result, error);
          return;
        }
        if (!cached && loaded) {
          // Changed while loading? Reopen securely and use direct reads.
          if (!file.open(root_, selected, error)) {
            file_error(result, error);
            return;
          }
          auto current = file.metadata(error);
          if (!current) {
            file_error(result, error);
            return;
          }
          metadata = *current;
        }
      }
    }
    if (cached) metadata = cached->metadata;
    const auto length = cached ? cached->bytes.size() : file.size();
    const auto etag =
        cached ? cached->etag
               : "W/\"" + std::to_string(metadata.device) + '-' +
                     std::to_string(metadata.identity) + '-' +
                     std::to_string(metadata.size) + '-' +
                     std::to_string(metadata.modified_seconds) + '-' +
                     std::to_string(metadata.modified_nanoseconds) + '-' +
                     std::to_string(metadata.changed_seconds) + '-' +
                     std::to_string(metadata.changed_nanoseconds) + "\"";
    const auto now = std::chrono::system_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch());
    const auto modified = std::min(metadata.modified_seconds, elapsed.count());
    const auto decorate = [this, &result, &etag, &cached, modified, &metadata] {
      result.set_header("ETag", etag);
      if (cached && modified == metadata.modified_seconds &&
          cached->last_modified[0])
        result.set_header("Last-Modified",
                          std::string_view(cached->last_modified.data()));
      else
        result.set_header("Last-Modified", detail::static_file_date(modified));
      if (!policies_.cache_control.empty())
        result.set_header("Cache-Control", policies_.cache_control);
      if (policies_.precompressed) result.set_header("Vary", "Accept-Encoding");
    };
    const auto match = field(req, "If-Match"),
               none = field(req, "If-None-Match");
    if (has_field(req, "If-Match")) {
      if (!detail::static_file_tag_matches(match, etag, true)) {
        result.precondition_failed_412();
        decorate();
        return;
      }
    } else if (auto date =
                   detail::static_file_time(field(req, "If-Unmodified-Since"));
               date && modified > *date) {
      result.precondition_failed_412();
      decorate();
      return;
    }
    if (has_field(req, "If-None-Match")) {
      if (detail::static_file_tag_matches(none, etag, false)) {
        result.not_modified_304();
        decorate();
        return;
      }
    } else if (auto date =
                   detail::static_file_time(field(req, "If-Modified-Since"));
               date && modified <= *date) {
      result.not_modified_304();
      decorate();
      return;
    }
    result.ok_200();
    std::size_t offset = 0, count = length;
    if (policies_.ranges) {
      // Ranges apply to GET; HEAD describes the full response.
      auto if_range = field(req, "If-Range");
      const bool allow = if_range.empty() ||
          (!etag.starts_with("W/") && if_range == etag);
      // A timestamp can't rule out two versions in one second. Send the full
      // body for a date-based If-Range.
      if (req.get_method() == "GET" && allow) {
        const auto range =
            detail::static_file_parse_range(field(req, "Range"), length);
        if (range.applicable) {
          if (!range.satisfiable) {
            result.range_not_satisfiable_416();
            decorate();
            result.set_header("Content-Range",
                              "bytes */" + std::to_string(length));
            return;
          }
          result.partial_content_206();
          offset = range.offset;
          count = range.length;
          result.set_header("Content-Range",
                            "bytes " + std::to_string(offset) + '-' +
                                std::to_string(offset + count - 1) + '/' +
                                std::to_string(length));
        }
      }
    }
    decorate();
    result.set_header("Content-Type", content_type(relative));
    if (coding != "identity") result.set_header("Content-Encoding", coding);
    if (policies_.ranges) result.set_header("Accept-Ranges", "bytes");
    if (req.get_method() == "HEAD") {
      result.set_header("Content-Length", std::to_string(length));
    } else if (cached) {
      result.set_body(common::reader::shared(
                          cached, std::as_bytes(std::span(cached->bytes.data(),
                                                          cached->bytes.size()))
                                      .subspan(offset, count)),
                      count);
    } else {
      if ((offset || count != length) && !file.select(offset, count, error)) {
        file_error(result, error);
        return;
      }
      result.set_body(common::reader(std::move(file)), count);
    }
  }
  // +=========================================================================+
  // | [>] content_type                                            ( private ) |
  // +-------------------------------------------------------------------------+
  std::string_view content_type(std::string_view path) const {
    const auto dot = path.rfind('.');
    if (dot == path.npos) return "application/octet-stream";
    const auto extension = path.substr(dot);
    for (const auto& type : policies_.mime_types)
      if (helpers::iequals(extension, type.first)) return type.second;
    constexpr std::pair<std::string_view, std::string_view> types[] = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "text/javascript"},
        {".json", "application/json"},
        {".txt", "text/plain"},
        {".svg", "image/svg+xml"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".webp", "image/webp"},
        {".ico", "image/vnd.microsoft.icon"},
        {".pdf", "application/pdf"},
        {".wasm", "application/wasm"},
        {".mjs", "text/javascript"}, {".map", "application/json"},
        {".xml", "application/xml"}, {".avif", "image/avif"},
        {".woff", "font/woff"}, {".woff2", "font/woff2"},
        {".ttf", "font/ttf"}, {".otf", "font/otf"},
        {".mp4", "video/mp4"}, {".webm", "video/webm"},
        {".mp3", "audio/mpeg"}, {".ogg", "audio/ogg"},
    };
    for (const auto& type : types) {
      if (helpers::iequals(extension, type.first)) return type.second;
    }
    return "application/octet-stream";
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::string prefix_;
  std::filesystem::path root_;
  static_file_policies policies_;
  std::string key_prefix_;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
