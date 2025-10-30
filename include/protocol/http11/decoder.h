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
    size_ = constants::limits::kDefaultCoreMsgMaxSizeInRam;
    buffer_ = (char*)malloc(size_);
    body_bytes_read_ = 0;
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
  inline bool add(const char* ptr, std::size_t size) {
    if ((size_ - cursor_) < size) {
      // ((error)): the size of the incoming buffer exceeds limits!
      return false;
    }
    memcpy(&buffer_[cursor_], ptr, size);
    cursor_ += size;
    return true;
  }
  inline std::shared_ptr<request> process() {
    static const auto eoh = (const char*)constants::string::kEndOfHeaders;
    static const auto eoh_len = sizeof(constants::string::kEndOfHeaders) - 1;
    // [first] let's detect the [end-of-headers] position..
    if (!request_) {
      std::string_view content(buffer_, cursor_);
      if (auto hdr = content.find(eoh); hdr != std::string_view::npos) {
        // try to create a valid request from incoming data..
        request_ = request::from(buffer_, cursor_);
        // now let's adjust internal buffer just by copying remaining bytes..
        if (cursor_ -= hdr + eoh_len) {
          memmove(buffer_, &buffer_[hdr + eoh_len], cursor_);
        }
      }
    }
    // [second] let's process the [bopy] part (if present)..
    if (request_) {
      auto body_length = request_->get_body_length();
      if (request_->has_body()) {
        if (body_length) {
          // [content-length] based body!
          std::size_t pending = body_length - body_bytes_read_;
          std::size_t to_grab = cursor_ >= pending ? pending : cursor_;
          request_->get_body_writer()->write(buffer_, to_grab);
          body_bytes_read_ += to_grab;
          cursor_ -= to_grab;
        } else {
          // [transfer-encoding:chunks] based body!
          // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          // [to-do] -> add support for this!
          // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        }
      }
      if (!request_->has_body() || body_bytes_read_ == body_length) {
        return std::move(request_);
      }
    }
    return nullptr;
  }
  inline void reset() {
    cursor_ = 0;
    body_bytes_read_ = 0;
    request_.reset();
  }

 private:
  // ___________________________________________________________________________
  // STATIC-ATTRIBUTEs                                               ( private )
  //
  char* buffer_;
  std::size_t size_;
  std::size_t cursor_;
  std::size_t body_bytes_read_;
  std::shared_ptr<request> request_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
