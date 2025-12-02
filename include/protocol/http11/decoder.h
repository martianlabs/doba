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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

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
    body_read_completed = false;
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
      if (request_->has_body()) {
        if (std::size_t body_length = request_->get_body_length()) {
          // [content-length] based body!
          std::size_t pending = body_length - body_bytes_read_;
          std::size_t to_grab = cursor_ >= pending ? pending : cursor_;

          /*
          pepe
          */

          /*
          request_->get_body_mutable()->write(buffer_, to_grab);
          */

          /*
          pepe fin
          */

          body_read_completed = (body_bytes_read_ += to_grab) == body_length;
          cursor_ -= to_grab;
        } else {
          // [transfer-encoding:chunks] based body!
          // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          // [to-do] -> add support for this!
          // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        }
      }
      if (!request_->has_body() || body_read_completed ) {
        body_bytes_read_ = 0;
        body_read_completed = false;
        return std::move(request_);
      }
    }
    return nullptr;
  }

 private:
  // ___________________________________________________________________________
  // STATIC-ATTRIBUTEs                                               ( private )
  //
  char* buffer_;
  std::size_t size_;
  std::size_t cursor_;
  bool body_read_completed;
  std::size_t body_bytes_read_;
  std::shared_ptr<request> request_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
