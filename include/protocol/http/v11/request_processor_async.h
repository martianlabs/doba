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

#ifndef martianlabs_doba_protocol_http_v11_request_processor_async_h
#define martianlabs_doba_protocol_http_v11_request_processor_async_h

#include <exception>
#include <memory>
#include <optional>
#include <utility>

#include "common/task_execution.h"
#include "protocol/processing_result.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/v11/decoder.h"

namespace martianlabs::doba::protocol::http::v11::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request_processor_async                                     ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
class request_processor_async {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  request_processor_async(decoder<RQty, RSty>& decoder, RQty request)
      : request_{std::move(request)}, response_{decoder.make_response()} {
    request_.make_owned();
  }
  request_processor_async(const request_processor_async&) = delete;
  request_processor_async(request_processor_async&&) = delete;
  ~request_processor_async() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  request_processor_async& operator=(const request_processor_async&) = delete;
  request_processor_async& operator=(request_processor_async&&) = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start(const router_match_result<RQty, RSty>& match) {
    auto work = match.async_handler
                    ? (*match.async_handler)(request_, response_)
                    : match.async_parametrized_handler->invoke(
                          request_, response_, request_.get_absolute_path(),
                          parameters_);
    execution_.emplace(std::move(work));
  }
  // +=========================================================================+
  // | [>] resume                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  bool resume(common::task_execution::clock::time_point now) {
    return execution_ && execution_->resume(now);
  }
  // +=========================================================================+
  // | [>] done                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool done() const noexcept { return !execution_ || execution_->done(); }
  // +=========================================================================+
  // | [>] error                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  std::exception_ptr error() const noexcept {
    return execution_ ? execution_->error() : nullptr;
  }
  // +=========================================================================+
  // | [>] processing                                               ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result processing() const noexcept {
    if (!execution_ || execution_->done()) return {};
    return {execution_->ready(), execution_->deadline(), false};
  }
  // +=========================================================================+
  // | [>] request                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  const RQty& request() const noexcept { return request_; }
  // +=========================================================================+
  // | [>] response                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  RSty& response() noexcept { return response_; }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  RQty request_;                    // Owned before handler invocation.
  RSty response_;                   // Independent of reusable sync storage.
  std::shared_ptr<void> parameters_;  // Typed route arguments, when needed.
  // Destroy the frame before its borrowed request/response/route arguments.
  std::optional<common::task_execution> execution_;
};
}  // namespace martianlabs::doba::protocol::http::v11::detail

#endif
