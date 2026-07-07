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


#ifndef martianlabs_doba_protocol_http11_connection_h
#define martianlabs_doba_protocol_http11_connection_h

#include <string_view>
#include <vector>

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] connection                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// | The hop-by-hop transport state that the semantic layer derives from a     |
// | request. Framing and connection interpreters/rules read the parsed values |
// | of Connection, Transfer-Encoding, TE, Trailer, and Upgrade and mutate     |
// | this state accordingly. It is deliberately separate from both the         |
// | agnostic deserialization result and the request itself, because it        |
// | outlives a single request (it describes the connection) and is HTTP/1.1-  |
// | specific.                                                                 |
// |                                                                           |
// | The std::string_view members are zero-copy and point back into the        |
// | request's field-value buffer; that buffer MUST outlive this state.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct connection {
  // Whether the connection is to be kept alive after the current message.
  // HTTP/1.1 defaults to persistent unless "Connection: close" is present.
  bool persistent = true;
  // Whether "Connection: close" was requested for this connection.
  bool close_requested = false;
  // The transfer-coding names of Transfer-Encoding, in order (outermost last).
  std::vector<std::string_view> transfer_codings;
  // Whether the final transfer-coding is "chunked" (message framing is then
  // delimited by the chunked coding rather than by Content-Length).
  bool chunked = false;
  // The transfer-coding names the client is willing to accept (TE), in order.
  std::vector<std::string_view> te_codings;
  // Whether the client signalled acceptance of the "trailers" TE token.
  bool accepts_trailers = false;
  // The header field names announced in the Trailer header, in order.
  std::vector<std::string_view> trailer_names;
  // The protocols offered in the Upgrade header, in order (empty when absent).
  std::vector<std::string_view> upgrade_offer;
  // Every connection-option token named by Connection, in order. These name
  // header fields that are hop-by-hop for this connection.
  std::vector<std::string_view> options;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
