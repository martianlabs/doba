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

#ifndef martianlabs_doba_protocol_http11_request_h
#define martianlabs_doba_protocol_http11_request_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// request                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 request implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class request {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  request() { buf_ = (char*)malloc(kMaxRequestLength); }
  request(const request&) = delete;
  request(request&&) noexcept = delete;
  ~request() { free(buf_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  request& operator=(const request&) = delete;
  request& operator=(request&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  transport::process_result deserialize(void* buffer, uint32_t length) {
    if ((kMaxRequestLength - cur_) < length) {
      return transport::process_result::kError;
    }
    memcpy(&buf_[cur_], buffer, length);
    cur_ += length;
    eoh_ = find_exact_match(buf_, cur_, kEndOfHeaders, kEndOfHeadersLen);
    if (!eoh_.has_value()) {
      return transport::process_result::kMoreBytesNeeded;
    }

    /*
    pepe
    */

    // printf("%.*s", int(buffer_cur_), buffer_);

    /*
    pepe fin
    */

    return transport::process_result::kCompleted;
  }
  void reset() {
    cur_ = 0;
    eoh_.reset();
  }

 private:
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  std::optional<uint32_t> find_exact_match(const char* const str,
                                           uint32_t str_len,
                                           const char* const pattern,
                                           uint32_t pattern_len) {
    for (uint32_t i = 0; i < str_len; ++i) {
      uint32_t j = 0;
      while (j < pattern_len) {
        if (i + j == str_len) return {};
        if (str[i + j] != pattern[j]) break;
        j++;
      }
      if (j == pattern_len) return i;
    }
    return {};
  }
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr uint32_t kMaxRequestLength = 4096;
  static constexpr char kEndOfHeaders[] = "\r\n\r\n";
  static constexpr uint32_t kEndOfHeadersLen = sizeof(kEndOfHeaders) - 1;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  char* buf_ = nullptr;
  uint32_t cur_ = 0;
  std::optional<uint32_t> eoh_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
