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

#ifndef martianlabs_doba_protocol_http11_body_common_h
#define martianlabs_doba_protocol_http11_body_common_h

#include <cstddef>
#include <cstdint>
#include <span>

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] TYPEs                                                                 |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
enum class body_status : std::uint8_t { open, complete, error };
enum class body_error : std::uint8_t {
  none,
  invalid_state,
  invalid_configuration,
  io_error,
  invalid_chunk_size,
  chunk_size_overflow,
  invalid_chunk_crlf,
  invalid_trailer,
  chunked_incomplete,
  raw_size_limit_exceeded,
  stored_size_limit_exceeded,
  source_size_limit_exceeded,
  chunk_extension_size_limit_exceeded,
  trailer_size_limit_exceeded
};
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] CONSTANTs                                                             |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
inline constexpr std::size_t kDefaultBufferSize = 8192;
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] BODY STORAGE                                                          |
// +--------------------------------------------------------------------------+
// | body_storage is the single, fixed storage backend used by body_writer and |
// | body_reader. It is not a template parameter: callers never choose the     |
// | storage type. Storage always starts in memory (std::string) and           |
// | transparently spills to a uniquely-named temporary file when the          |
// | accumulated wire bytes exceed body_storage_options::spill_threshold       |
// | (0 = never spill). The transition is invisible to the codec and to the    |
// | serializer/deserializer.                                                  |
// |                                                                           |
// | The decode cursor (fetch()/read()) and the wire cursor (wire_read()) are  |
// | independent and can be driven simultaneously without interfering.         |
// |                                                                           |
// | The internal interface honoured by body_storage (reproduced here for      |
// | reference; body_storage.h is the authoritative source):                  |
// |                                                                           |
// | sink (body_writer, driven by body_serializer<CDty>):                      |
// |   bool write(const char* ptr, std::size_t len)                            |
// |     -> append len wire bytes; return false on I/O failure.               |
// |   void on_finish(std::uint64_t stored)                                    |
// |     -> finalise the backend (flush if spilled, record readable size).    |
// |                                                                           |
// | source (body_reader, driven by body_deserializer<CDty>):                  |
// |   bool fetch(std::byte& b)                                                |
// |     -> read one byte at the decode cursor and advance it; false at EOF.  |
// |   std::size_t read(char* out, std::size_t want)                           |
// |     -> bulk read at the decode cursor; advance it; return count.         |
// |   std::size_t wire_read(char* out, std::size_t want)                      |
// |     -> read at the independent wire cursor; advance it; return count.    |
// |   std::optional<std::uint64_t> total_size() const noexcept               |
// |     -> known total wire size, or nullopt if not yet finalised.           |
// |   bool exhausted() const noexcept                                         |
// |     -> true when the decode cursor has reached the end.                  |
// |   bool ok() const noexcept                                                |
// |     -> false once the backend has hit an unrecoverable I/O error.        |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] encode_result                                              ( struct ) |
// +---------------------------------------------------------------------------+
// | Reported by a codec's encode()/finish() so the writer can keep its own    |
// | stored-size accounting and enforce its stored-size limit.                 |
// +===========================================================================+
struct encode_result {
  // Bytes actually written to the sink for this call (raw: == input length;
  // chunked: chunk-frame overhead + input length; finish adds the terminator).
  std::uint64_t stored = 0;
  // True when a framing failure occurred; error carries the reason. A sink I/O
  // failure leaves this false (the writer maps it to io_error).
  bool has_error = false;
  body_error error = body_error::none;
};
// +===========================================================================+
// | [>] decode_result                                              ( struct ) |
// +---------------------------------------------------------------------------+
// | Reported by a codec's decode() so the reader can keep its own logical     |
// | byte accounting, enforce its raw-size limit and map completion/errors to  |
// | body_status/body_error.                                                   |
// +===========================================================================+
struct decode_result {
  // Logical (post-decoding) bytes written to the output buffer.
  std::size_t produced = 0;
  // True once the codec has fully consumed the encoded body.
  bool complete = false;
  // True when decoding failed; error carries the reason.
  bool has_error = false;
  body_error error = body_error::none;
};
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] BODY CODEC CONTRACT                                                    |
// +--------------------------------------------------------------------------+
// | A body codec (CDty) is the wire-structure transformer plugged into        |
// | body_serializer<CDty> (encode side) and body_deserializer<CDty> (decode  |
// | side). It moves the raw/chunked framing decision to a compile-time        |
// | template parameter so extra encodings can be added without touching the   |
// | serializer/deserializer orchestration. The bundled codecs live in         |
// | body_codec_raw.h and body_codec_chunked.h.                                |
// |                                                                           |
// | The codec owns any framing/decoding state (e.g. the chunked decoder state |
// | machine); the serializer/deserializer own their public byte counters,     |
// | their size limits and the storage. The codec only drives the decode       |
// | cursor (fetch()/read()) of the storage, never the wire cursor.            |
// |                                                                           |
// | encode side (body_serializer<CDty>):                                      |
// |   bool encode(body_writer& sink, const char* data, std::size_t len,      |
// |               encode_result& out)                                         |
// |     -> frame len bytes into sink; report bytes stored in out.stored;      |
// |        return false on failure. On a framing failure set out.has_error/   |
// |        out.error; on a sink I/O failure leave out.has_error false (the    |
// |        serializer maps it to io_error). Empty writes are ok.              |
// |   bool finish(body_writer& sink, encode_result& out)                      |
// |     -> emit any trailing framing (raw: none; chunked: "0\r\n\r\n");       |
// |        report bytes stored in out.stored; same failure convention.        |
// |                                                                           |
// | decode side (body_deserializer<CDty>):                                    |
// |   decode_result decode(body_reader& src, std::span<std::byte> output,    |
// |                        std::uint64_t& source_consumed)                    |
// |     -> pull encoded bytes from src, decode into output, advance internal  |
// |        state; add the number of source bytes read to source_consumed;     |
// |        report produced/complete/has_error/error in the result.            |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
}  // namespace martianlabs::doba::protocol::http11

#endif
