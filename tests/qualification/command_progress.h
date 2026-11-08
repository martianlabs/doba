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

#ifndef martianlabs_doba_tests_qualification_command_progress_h
#define martianlabs_doba_tests_qualification_command_progress_h

#include "../helpers/tool_process.h"

namespace martianlabs::doba::tests::tools {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] command_progress                                            ( class ) |
// +---------------------------------------------------------------------------+
// | Watch one child without extra threads or rereading whole files. Stream    |
// | readable logs; leave LLVM exports in their files.                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class command_progress {
 private:
  // +-------------------------------------------------------------------------+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  process& child_;
  std::string name_;
  clock::time_point started_{clock::now()};
  std::ofstream log_;
  std::ifstream output_;
  std::ifstream errors_;

  // +-------------------------------------------------------------------------+
  // | [>] drain                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void drain() {
    if (!log_.is_open()) return;
    for (auto* input : {&output_, &errors_}) {
      input->clear();
      char data[8192];
      while (input->read(data, sizeof(data)) || input->gcount()) {
        auto count = input->gcount();
        log_.write(data, count);
        std::cout.write(data, count);
      }
      require(!input->bad(), "Cannot read running child output");
    }
    log_.flush();
    std::cout.flush();
    require(static_cast<bool>(log_), "Cannot write running child log");
  }

  // +-------------------------------------------------------------------------+
  // | [>] elapsed                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  double elapsed() const {
    return std::chrono::duration<double>(clock::now() - started_).count();
  }

 public:
  // +-------------------------------------------------------------------------+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  command_progress(process& child, std::string name,
                   const fs::path& log = {}) :
    child_(child), name_(std::move(name)) {
    if (!log.empty()) {
      log_.open(log, std::ios::binary);
      output_.open(child.output_path(), std::ios::binary);
      errors_.open(child.error_path(), std::ios::binary);
      require(log_.is_open() && output_.is_open() && errors_.is_open(),
        "Cannot open command progress log");
    }
    std::cout << "[coverage] " << name_ << " started" << std::endl;
  }

  // +-------------------------------------------------------------------------+
  // | [>] wait                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  int wait(clock::duration timeout) {
    auto deadline = started_ + timeout;
    auto next = started_ + 15s;
    auto next_output = started_;
    try {
      while (!child_.poll()) {
        auto now = clock::now();
        if (now >= next_output) {
          drain();
          next_output = now + 100ms;
        }
        require(now < deadline, "Child deadline exceeded: " + name_);
        if (now >= next) {
          std::cout << "[coverage] " << name_ << " running: " <<
            elapsed() << " seconds" << std::endl;
          next = now + 15s;
        }
        std::this_thread::sleep_for(10ms);
      }
      drain();
      int code = child_.wait(0s);
      std::cout << "[coverage] " << name_ << " finished: " << elapsed() <<
        " seconds; exit " << code << std::endl;
      return code;
    } catch (...) {
      child_.kill();
      child_.wait(5s);
      drain();
      if (log_.is_open()) {
        log_ << "\nCommand failed or deadline exceeded\n";
        log_.flush();
      }
      std::cout << "[coverage] " << name_ << " failed after " << elapsed() <<
        " seconds" << std::endl;
      throw;
    }
  }
};

} // namespace martianlabs::doba::tests::tools

#endif
