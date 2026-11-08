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

#ifndef martianlabs_doba_transport_server_tcp_windows_overlapped_processing_h
#define martianlabs_doba_transport_server_tcp_windows_overlapped_processing_h

#include <memory>
#include <utility>

#include "protocol/contracts.h"
#include "transport/server/tcp_windows_overlapped_base.h"

namespace martianlabs::doba::transport::server::detail {
template <protocol::contracts::engine ENty>
struct context;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_processing                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | One immediate turn. Establish ownership before posting to IOCP.           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct overlapped_processing : overlapped_base {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit overlapped_processing(std::shared_ptr<context<ENty>> owner)
      : overlapped_base(io_type::kProcessing), ctx{std::move(owner)} {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Keep this exact connection alive until the queued packet is delivered.
  std::shared_ptr<context<ENty>> ctx;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
