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

#include "common/console_logger.h"

using namespace martianlabs::doba::common;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | This is the entry point of the console logger example. It demonstrates    |
// | how to use the console_logger class to log messages with different        |
// | severity levels. The logger is configured to not show timestamps,         |
// | function names, or line numbers in the log output. The main function logs |
// | an informational message indicating that the server has started,          |
// | followed by a warning message as an example. Finally, it returns 0 to     |
// | indicate successful execution.                                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  console_logger logger(
      "common",
      {.show_timestamp = false, .show_function = false, .show_line = false});
  logger.info("server started");
  logger.warning("example warning");
  return 0;
}
