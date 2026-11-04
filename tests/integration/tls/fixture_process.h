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
// Copyright 2026 martianLabs
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

#ifndef martianlabs_doba_tests_integration_tls_fixture_process_h
#define martianlabs_doba_tests_integration_tls_fixture_process_h

#include "../../helpers/tool_peer.h"
#include "../../helpers/tool_process.h"

namespace martianlabs::doba::tests::tools {

// +---------------------------------------------------------------------------+
// | [>] fixture_process                                             ( class ) |
// +---------------------------------------------------------------------------+
class fixture_process {
  uint16_t port_{peer::available_port()};
  process child_;
  bool stopped_{};

 public:
  // +-------------------------------------------------------------------------+
  // | [>] PUBLIC INTERFACE                                       ( public ) |
  // +-------------------------------------------------------------------------+
  fixture_process(const std::string& binary, const std::string& certificate,
    const std::string& key, const std::string& mode = "echo",
    size_t plain = 1048576, size_t bio = 17408, size_t cipher = 2097152) :
    child_({binary, certificate, key, std::to_string(port_), mode,
      std::to_string(plain), std::to_string(bio), std::to_string(cipher)}) {
    child_.expect_output("READY\n");
  }
  fixture_process(const fixture_process&) = delete;
  fixture_process& operator=(const fixture_process&) = delete;
  ~fixture_process() {
    if (!stopped_) {
      try { stop(); }
      catch (const std::exception& error) {
        // Fail even during exception unwinding; a broken cleanup cannot pass.
        std::cerr << "TLS fixture cleanup failed: " << error.what() << '\n';
        std::abort();
      }
    }
  }
  uint16_t port() const { return port_; }
  process& child() { return child_; }
  void stop() {
    if (stopped_) return;
    if (!child_.poll()) child_.send("stop\n");
    int code = child_.wait(10s);
    clean_diagnostics(child_.errors());
    require(code == 0, "TLS fixture failed: " + child_.errors());
    stopped_ = true;
  }
};

} // namespace martianlabs::doba::tests::tools

#endif
