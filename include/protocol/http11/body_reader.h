// +---------------------------+
// |  d o b a                  |
// +---------------------------+
//
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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License").
// You may obtain a copy of the License at:
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef martianlabs_doba_protocol_http11_body_reader_h
#define martianlabs_doba_protocol_http11_body_reader_h

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

#include "protocol/http11/body_common.h"
#include "protocol/http11/body_storage.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] body_reader                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Opaque byte source over body_storage. No codec knowledge: reads raw bytes |
// | from the storage backend sequentially.                                    |
// | body_deserializer<CDty> owns the codec and drives body_reader as its src. |
// | response::serialize() also uses body_reader directly to drain a           |
// | body_serializer's released storage to the wire sink.                     |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
class body_reader {
 public:
  body_reader() = default;
  explicit body_reader(body_storage storage) : storage_(std::move(storage)) {}
  body_reader(const body_reader&) = delete;
  body_reader& operator=(const body_reader&) = delete;
  body_reader(body_reader&&) noexcept = default;
  body_reader& operator=(body_reader&&) noexcept = default;

  // Reads up to output.size() raw bytes. Returns 0 at end-of-storage or on
  // I/O error (check failed() to distinguish).
  std::size_t read(std::span<std::byte> output) {
    if (output.empty()) return 0;
    std::size_t n = storage_.read(reinterpret_cast<char*>(output.data()),
                                  output.size());
    if (n == 0 && !storage_.ok()) failed_ = true;
    return n;
  }

  // fetch() for codec use: reads one byte via the decode cursor.
  bool fetch(std::byte& b) {
    if (!storage_.fetch(b)) {
      if (!storage_.ok()) failed_ = true;
      return false;
    }
    return true;
  }

  [[nodiscard]] bool eof() const noexcept { return storage_.exhausted(); }
  [[nodiscard]] bool failed() const noexcept { return failed_; }
  [[nodiscard]] bool ok() const noexcept { return storage_.ok(); }
  // exhausted() is an alias for eof(), used by codec internals.
  [[nodiscard]] bool exhausted() const noexcept { return storage_.exhausted(); }
  [[nodiscard]] std::optional<std::uint64_t> size() const noexcept {
    return storage_.total_size();
  }

  // Reads all remaining bytes into out. Returns total bytes appended.
  std::size_t read_all(std::string& out) {
    std::byte buf[kDefaultBufferSize];
    std::size_t total = 0;
    while (!eof() && !failed()) {
      std::size_t n = read(std::span<std::byte>(buf, sizeof(buf)));
      if (n == 0) break;
      out.append(reinterpret_cast<const char*>(buf), n);
      total += n;
    }
    return total;
  }

 private:
  template <typename CDty> friend class body_serializer;
  template <typename CDty> friend class body_deserializer;

  [[nodiscard]] body_storage release() { return std::move(storage_); }

  body_storage storage_;
  bool failed_{false};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
