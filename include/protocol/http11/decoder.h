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

#ifndef martianlabs_doba_protocol_http11_decoder_h
#define martianlabs_doba_protocol_http11_decoder_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// decoder                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 server decoder implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class decoder {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  decoder() {
    cursor_ = 0;
    size_ = constants::limits::kDefaultRequestMaxSize;
    buf_ = (char*)malloc(size_);
  }
  decoder(const decoder&) = delete;
  decoder(decoder&&) noexcept = delete;
  ~decoder() { free(buf_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  decoder& operator=(const decoder&) = delete;
  decoder& operator=(decoder&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  bool add(const char* ptr, std::size_t size) {
    if ((size_ - cursor_) < size) {
      // ((error)): the size of the incoming buffer exceeds limits!
      return false;
    }
    memcpy(&buf_[cursor_], ptr, size);
    cursor_ += size;
    return true;
  }
  std::shared_ptr<request> process() {
    static const auto eoh = (const char*)constants::string::kEndOfHeaders;
    static const auto eoh_len = sizeof(constants::string::kEndOfHeaders) - 1;
    if (!request_) {
      // let's detect the [end-of-headers] position..
      std::string_view content(buf_, cursor_);
      if (auto hdr = content.find(eoh); hdr != std::string_view::npos) {
        if (request_ = request::from(buf_, cursor_)) {
        }
        cursor_ -= hdr + eoh_len;
      }
    }
    return std::move(request_);
  }
  void reset() { cursor_ = 0; }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  // ___________________________________________________________________________
  // STATIC-ATTRIBUTEs                                               ( private )
  //
  char* buf_;
  std::size_t size_;
  std::size_t cursor_;
  std::shared_ptr<request> request_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
