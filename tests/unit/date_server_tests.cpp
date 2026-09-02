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

#include <array>
#include <atomic>
#include <string_view>
#include <thread>
#include <vector>

#include "common/date_server.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::date_server;
using martianlabs::doba::protocol::http::v11::response;

bool valid_http_date(std::string_view value) {
  return value.size() == 29 && value[3] == ',' && value[4] == ' ' &&
         value[7] == ' ' && value[11] == ' ' && value[16] == ' ' &&
         value[19] == ':' && value[22] == ':' && value[25] == ' ' &&
         value.substr(26) == "GMT";
}
}  // namespace

// +===========================================================================+
// | [>] concurrent serialization preserves HTTP dates           ( test-case ) |
// +===========================================================================+
DOBA_TEST("concurrent serialization preserves HTTP dates") {
  date_server::get().start();
  std::atomic<bool> valid{true};
  std::vector<std::thread> threads;
  for (std::size_t thread = 0; thread < 4; ++thread) {
    threads.emplace_back([&valid] {
      for (std::size_t index = 0; index < 10000; ++index) {
        response value;
        auto serialized = value.serialize();
        const std::size_t begin = serialized->prefix.find("Date: ");
        if (begin == std::string::npos ||
            !valid_http_date(serialized->prefix.substr(begin + 6, 29))) {
          valid.store(false);
          return;
        }
      }
    });
  }
  for (auto& thread : threads) thread.join();
  date_server::get().stop();
  DOBA_EXPECT(valid.load());
}
