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

#ifndef martianlabs_doba_protocol_http11_body_codec_raw_h
#define martianlabs_doba_protocol_http11_body_codec_raw_h

#include <cstddef>
#include <cstdint>
#include <span>

#include "protocol/http11/body_common.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] body_codec_raw                                               ( class ) |
// +---------------------------------------------------------------------------+
// | Identity body codec: bytes cross unchanged in both directions. Fulfils the |
// | BODY CODEC CONTRACT declared in body_common.h so it can be plugged into    |
// | body_writer<body_codec_raw> and body_reader<body_codec_raw>.                |
// |                                                                           |
// | encode : input bytes are stored verbatim (stored == input length).        |
// |          finish() is a no-op (raw bodies carry no trailing framing).      |
// | decode : bytes are pulled verbatim from the source; completion is driven  |
// |          by the backend reaching its end.                                 |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
class body_codec_raw {
 public:
  // +=========================================================================+
  // | [>] encode                                                   ( public ) |
  // | ------------------------------------------------------------------------+
  // | Stores len bytes verbatim into the sink. Reports the stored count.      |
  // +=========================================================================+
  template <typename BDty>
  bool encode(BDty& sink, const char* data, std::size_t len,
              encode_result& out) {
    out.stored = 0;
    if (len == 0) return true;
    if (!sink.write(data, len)) return false;
    out.stored = static_cast<std::uint64_t>(len);
    return true;
  }
  // +=========================================================================+
  // | [>] finish                                                   ( public ) |
  // | ------------------------------------------------------------------------+
  // | Raw bodies have no terminator; nothing is written.                      |
  // +=========================================================================+
  template <typename BDty>
  bool finish(BDty& sink, encode_result& out) {
    (void)sink;
    out.stored = 0;
    return true;
  }
  // +=========================================================================+
  // | [>] decode                                                   ( public ) |
  // | ------------------------------------------------------------------------+
  // | Reads bytes verbatim from the source into output. Marks completion once |
  // | the backend is exhausted; surfaces backend I/O faults as io_error.      |
  // +=========================================================================+
  template <typename BDty>
  decode_result decode(BDty& src, std::span<std::byte> output,
                       std::uint64_t& source_consumed) {
    decode_result r;
    std::size_t n = src.read(output);
    if (n == 0) {
      if (!src.ok()) {
        r.has_error = true;
        r.error = body_error::io_error;
        return r;
      }
      r.complete = true;
      return r;
    }
    r.produced = n;
    source_consumed += n;
    if (src.exhausted()) r.complete = true;
    return r;
  }
};
}  // namespace martianlabs::doba::protocol::http11

#endif
