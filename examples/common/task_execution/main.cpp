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

#include <chrono>
#include <iostream>
#include <thread>

#include "common/task_execution.h"

using namespace martianlabs::doba::common;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] work                                                     ( function ) |
// +---------------------------------------------------------------------------+
// | yield() and sleep_for() pause the task without starting threads.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
task<> work(int& steps) {
  ++steps;
  co_await yield();
  ++steps;
  co_await sleep_for(std::chrono::milliseconds(1));
  ++steps;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | The caller waits for deadlines and resumes the task.                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  int steps = 0;
  task_execution execution(work(steps));
  while (!execution.done()) {
    if (auto due = execution.deadline()) std::this_thread::sleep_until(*due);
    execution.resume();
  }
  if (execution.error()) return 1;
  execution.cancel();
  std::cout << "Completed steps: " << steps << '\n';
  return steps == 3 ? 0 : 1;
}
