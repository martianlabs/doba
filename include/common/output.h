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
// Concurrent submissions are serialized; engines determine protocol order.
// The source follows the buffer; later deliveries cannot overtake it.
// Output queued during an input callback starts after that callback returns.
// Other submissions activate output without requiring new input.
// Empty deliveries without a source reserve no bytes.
// Submissions after closing are ignored, including calls through a retained
// delegate after transport destruction.
// Sources initially reserve up to 8192 bytes, or their prefix size if larger.
// Queued reservations and the entire active batch share the send capacity.
// Source-owned storage is outside this limit; borrowed storage must stay valid
// until delivery finishes or is discarded.
// Invalid deliveries or overflow close the connection; rejected data is freed.
// The transport never calls back into the engine while sending.
using send_delegate =
    std::function<void(std::unique_ptr<char[]>, std::size_t,
                       std::optional<reader>)>;
}  // namespace martianlabs::doba::common

#endif
