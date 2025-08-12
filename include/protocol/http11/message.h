//      _       _
//   __| | ___ | |__   __ _
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
//
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_protocol_http11_message_h
#define martianlabs_doba_protocol_http11_message_h

#include "hash_set.h"
#include "headers.h"
#include "body.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// message                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 request/response base class.
// -----------------------------------------------------------------------------
// =============================================================================
class message {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  message() = default;
  message(const message&) = delete;
  message(message&&) noexcept = delete;
  ~message() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  message& operator=(const message&) = delete;
  message& operator=(message&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  inline message& prepare(char* headers_buffer, std::size_t headers_buffer_size,
                          char* body_buffer, std::size_t body_buffer_size,
                          std::size_t headers_buffer_used) {
    headers_.prepare(headers_buffer, headers_buffer_size, headers_buffer_used);
    body_.prepare(body_buffer, body_buffer_size);
    return *this;
  }
  inline void reset() {
    headers_.reset();
    body_.reset();
  }
  inline message& add_header(std::string_view k, std::string_view v) {
    headers_.add(k, v);
    return *this;
  }
  template <typename T>
    requires std::is_arithmetic_v<T>
  inline message& add_header(std::string_view k, const T& v) {
    return add_header(k, std::to_string(v));
  }
  inline hash_map<std::string_view, std::string_view> get_headers() const {
    return headers_.get_all();
  }
  inline std::optional<std::string_view> get_header(std::string_view k) const {
    return headers_.get(k);
  }
  inline message& remove_header(std::string_view key) {
    headers_.remove(key);
    return *this;
  }
  inline message& add_body(const char* buffer, std::size_t size) {
    body_.add(buffer, size);
    return *this;
  }
  inline message& add_body(std::string_view sv) {
    body_.add(sv.data(), sv.size());
    return *this;
  }
  inline message& add_hop_by_hop_header(std::string_view hop) {
    hop_by_hop_headers_.insert(std::string(hop));
    return *this;
  }
  inline hash_set<std::string> get_hop_by_hop_headers() const {
    return hop_by_hop_headers_;
  }
  inline message& clear_hop_by_hop_headers() {
    hop_by_hop_headers_.clear();
    return *this;
  }
  inline std::size_t get_headers_length() const { return headers_.length(); }
  inline std::size_t get_body_length() const { return body_.length(); }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  headers headers_;
  body body_;
  hash_set<std::string> hop_by_hop_headers_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
