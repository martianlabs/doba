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

#ifndef martianlabs_doba_protocol_http11_transfer_encodings_h
#define martianlabs_doba_protocol_http11_transfer_encodings_h

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] transfer_encodings                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 supported transfer encodings.           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct transfer_encodings {
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +=========================================================================+
  // +------------------------+------------------------------------------------+
  // | Encoding               | Description                                    |
  // +------------------------+------------------------------------------------+
  // | chunked                | What "chunked" is                              |
  // |                        |  - "chunked" is a transfer-coding used to      |
  // |                        |    frame a message body as a sequence of       |
  // |                        |    chunks, each preceded by its size, and      |
  // |                        |    terminated by a zero-size chunk,            |
  // |                        |    optionally followed by a trailer section.   |
  // |                        |  - It is framing for transfer;                 |
  // |                        |    it is not a content coding.                 |
  // |                        |                                                |
  // |                        | Position in Transfer-Encoding                  |
  // |                        |  - If any transfer coding other than "chunked" |
  // |                        |    is applied, it MUST be applied before       |
  // |                        |    "chunked". Therefore, "chunked" MUST be     |
  // |                        |    the last transfer-coding in the             |
  // |                        |    Transfer-Encoding field value.              |
  // |                        |                                                |
  // |                        | Interaction with Content-Length                |
  // |                        |  - A sender MUST NOT send Content-Length       |
  // |                        |    in any message that contains                |
  // |                        |    Transfer-Encoding.                          |
  // |                        |  - A recipient MUST ignore Content-Length      |
  // |                        |    when Transfer-Encoding is present.          |
  // |                        |                                                |
  // |                        | Message body format when chunked is used       |
  // |                        |  - The body is transmitted as:                 |
  // |                        |      chunk-size CRLF                           |
  // |                        |      chunk-data CRLF                           |
  // |                        |      ...                                       |
  // |                        |      0 CRLF                                    |
  // |                        |      trailer-section CRLF                      |
  // |                        |                                                |
  // |                        |  - chunk-size is a hexadecimal number (in      |
  // |                        |    ASCII), optionally followed by chunk        |
  // |                        |    extensions (after ';').                     |
  // |                        |                                                |
  // |                        | End of the message body                        |
  // |                        |  - With "chunked", the end of the body is      |
  // |                        |    indicated by the zero-size chunk and the    |
  // |                        |    terminating CRLF sequence, not by EOF and   |
  // |                        |    not by Content-Length.                      |
  // |                        |                                                |
  // |                        | Trailers                                       |
  // |                        |  - After the final (zero-size) chunk, a        |
  // |                        |    trailer section may appear.                 |
  // |                        |  - Trailers are header fields transferred      |
  // |                        |    after the body; recipients must parse       |
  // |                        |    them correctly if present (policy may       |
  // |                        |    restrict which are used).                   |
  // +------------------------+------------------------------------------------+
  static constexpr char kChunked[] = "chunked";
};
}  // namespace martianlabs::doba::protocol::http11

#endif
