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

#include <memory>
#include <optional>

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
// | [>] deserialization_result                                     ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct holds the overall result on protocol deserialization.         |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
struct deserialization_result {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  deserialization_result() : code(deserialization_status::kInvalidSource) {}
  deserialization_result(deserialization_status code) : code(code) {}
  deserialization_result(std::shared_ptr<RQty> request)
      : code(deserialization_status::kSucceeded), request(request) {}
  deserialization_result(const deserialization_result&) = default;
  deserialization_result(deserialization_result&&) = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  deserialization_result& operator=(const deserialization_result&) = default;
  deserialization_result& operator=(deserialization_result&&) = default;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  deserialization_status code = deserialization_status::kInvalidSource;
  std::shared_ptr<RQty> request = nullptr;
  std::optional<RSty> response;
};
}  // namespace martianlabs::doba::protocol

#endif
