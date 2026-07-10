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

#ifndef martianlabs_doba_protocol_http11_body_writer_h
#define martianlabs_doba_protocol_http11_body_writer_h

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "protocol/http11/body_storage.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] body_writer                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Opaque byte sink over body_storage. No codec knowledge: appends raw bytes.|
// | body_serializer<CDty> owns the codec and drives body_writer as its sink.  |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
class body_writer {
 public:
  body_writer() = default;
  explicit body_writer(body_storage_options opts) : storage_(std::move(opts)) {}
  body_writer(const body_writer&) = delete;
  body_writer& operator=(const body_writer&) = delete;
  body_writer(body_writer&&) noexcept = default;
  body_writer& operator=(body_writer&&) noexcept = default;

  bool write(const char* ptr, std::size_t len) {
    return storage_.write(ptr, len);
  }
  bool write(std::string_view sv) {
    return storage_.write(sv.data(), sv.size());
  }
  bool write(std::span<const std::byte> data) {
    return storage_.write(reinterpret_cast<const char*>(data.data()),
                          data.size());
  }
  // total_stored: byte count tracked by body_serializer (includes framing).
  void finish(std::uint64_t total_stored) { storage_.on_finish(total_stored); }
  [[nodiscard]] bool ok() const noexcept { return storage_.ok(); }
  [[nodiscard]] body_storage release() { return std::move(storage_); }

 private:
  body_storage storage_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
