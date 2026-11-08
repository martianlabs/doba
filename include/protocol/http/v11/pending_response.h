//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef martianlabs_doba_protocol_http_v11_pending_response_h
#define martianlabs_doba_protocol_http_v11_pending_response_h

#include <memory>
#include <string>

#include "common/reader.h"
#include "protocol/http/v11/request_processor_async.h"

namespace martianlabs::doba::protocol::http::v11::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] pending_response                                           ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
struct pending_response {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Only async handlers need to retain requests and coroutine frames.
  std::unique_ptr<request_processor_async<RQty, RSty>> handler;
  std::string interim;   // Keep at most one ordered 100 Continue per request.
  std::string head;      // Own the head until transport acceptance.
  std::string body;      // Own the inline body until the transport accepts it.
  std::unique_ptr<common::reader> source;  // Same reader rules as sync.
  bool complete{false};  // Final response ready to send in order.
  bool close{false};     // Close when this reply reaches the wire front.
  bool source_sent{false};  // Keep owner until the terminal reader drains.
};
}  // namespace martianlabs::doba::protocol::http::v11::detail

#endif
