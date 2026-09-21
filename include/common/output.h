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

#ifndef martianlabs_doba_common_output_h
#define martianlabs_doba_common_output_h

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>

#include "common/reader.h"

namespace martianlabs::doba::common {
// A buffer and optional source are moved together into the connection's FIFO.
// The source follows the buffer; later deliveries cannot overtake it.
// Each delivery produces one on_send_completed(bool) call on an I/O worker.
// True means fully written locally; false means failure or cancellation.
// Sources reserve up to 8192 bytes of send capacity for a reusable read buffer,
// or their prefix size if larger. Source-owned storage is outside this limit.
// Borrowed source storage must remain valid until completion or cancellation.
// Invalid deliveries or overflow close the connection; rejected data is freed.
// Invoke only from an engine callback, during the connection's lifetime.
// Engine callbacks are serialized and never reentered by this delegate.
using send_delegate =
    std::function<void(std::unique_ptr<char[]>, std::size_t,
                       std::optional<reader>)>;
}  // namespace martianlabs::doba::common

#endif
