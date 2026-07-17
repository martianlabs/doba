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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_protocol_deserialization_h
#define martianlabs_doba_protocol_deserialization_h

namespace martianlabs::doba::protocol {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] deserialization_status                                 ( enum-class ) |
// +---------------------------------------------------------------------------+
// | This enum class holds the result code on protocol deserialization.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class deserialization_status {
  kSucceeded,        // everything went fine.
  kInvalidSource,    // source data is invalid.
  kMoreBytesNeeded,  // more bytes are needed to perform de-serialization.
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] channel_intent                                         ( enum-class ) |
// +---------------------------------------------------------------------------+
// | The closed, protocol-agnostic vocabulary a protocol uses to tell the      |
// | transport what to do with the underlying channel after a message has been |
// | processed. These are the only things any transport can physically do with |
// | a channel, so they are universal: they belong to the channel, not to any  |
// | particular protocol or transport.                                         |
// +---------------------------------------------------------------------------+
// | kKeep    - keep the channel open (the transport arms another receive).    |
// | kClose   - close the channel once the pending response has been sent.     |
// | kUpgrade - hand the channel off (the transport stops governing it as-is). |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class channel_intent {
  kKeep,     // keep the channel open (arm another receive).
  kClose,    // close the channel once the response has been sent.
  kUpgrade,  // hand the channel off (stop governing it as-is).
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] deserialization_result                                     ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct holds the overall result on protocol deserialization.         |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty>
struct deserialization_result {
  deserialization_result() : code(deserialization_status::kInvalidSource) {}
  deserialization_result(deserialization_status code) : code(code) {}
  deserialization_result(std::shared_ptr<RQty> request, std::size_t bytes_used,
                         channel_intent channel = channel_intent::kKeep)
      : code(deserialization_status::kSucceeded),
        request(request),
        bytes_used(bytes_used),
        channel(channel) {}
  deserialization_status code = deserialization_status::kInvalidSource;
  std::shared_ptr<RQty> request = nullptr;
  std::size_t bytes_used = 0;
  channel_intent channel = channel_intent::kKeep;
};
}  // namespace martianlabs::doba::protocol

#endif
