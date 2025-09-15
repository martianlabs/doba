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

#include "body_writer.h"

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
    size_ = constants::limits::kDefaultCoreMsgMaxSizeInRam;
    buffer_ = (char*)malloc(size_);
  }
  decoder(const decoder&) = delete;
  decoder(decoder&&) noexcept = delete;
  ~decoder() { free(buffer_); }
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
    memcpy(&buffer_[cursor_], ptr, size);
    cursor_ += size;
    return true;
  }
  std::shared_ptr<request> process() {
    static const auto eoh = (const char*)constants::string::kEndOfHeaders;
    static const auto eoh_len = sizeof(constants::string::kEndOfHeaders) - 1;
    if (!request_) {
      // let's detect the [end-of-headers] position..
      std::string_view content(buffer_, cursor_);
      if (auto hdr = content.find(eoh); hdr != std::string_view::npos) {
        request_ = request::from(buffer_, cursor_, exp_bdy_, exp_bdy_len_);
        if (exp_bdy_) body_writer_ = body_writer::memory_writer(exp_bdy_len_);
        if (cursor_ -= hdr + eoh_len) {
          memmove(buffer_, &buffer_[hdr + eoh_len], cursor_);
        }
        cur_bdy_len_ = 0;
      }
    }
    if (body_writer_) {
      if (exp_bdy_len_) {
        // [content-length] based body!
        std::size_t pending = exp_bdy_len_ - cur_bdy_len_;
        std::size_t to_grab = cursor_ >= pending ? pending : cursor_;
        body_writer_->write(buffer_, to_grab);
        cur_bdy_len_ += to_grab;
        cursor_ -= to_grab;
      } else {
        // [transfer-encoding:chunks] based body!
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        // [to-do] -> add support for this!
        // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      }
    }
    return !exp_bdy_ || cur_bdy_len_ == exp_bdy_len_ ? std::move(request_)
                                                     : nullptr;
  }
  void reset() {
    cursor_ = 0;
    exp_bdy_ = false;
    exp_bdy_len_ = 0;
    cur_bdy_len_ = 0;
    request_.reset();
    body_writer_.reset();
  }

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
  char* buffer_;
  std::size_t size_;
  std::size_t cursor_;
  bool exp_bdy_;
  std::size_t exp_bdy_len_;
  std::size_t cur_bdy_len_;
  std::shared_ptr<request> request_;
  std::shared_ptr<body_writer> body_writer_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
