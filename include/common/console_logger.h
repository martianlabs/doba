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

#ifndef martianlabs_doba_common_console_logger_h
#define martianlabs_doba_common_console_logger_h

#include <source_location>
#include <string>
#include <string_view>

#include "platform.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] console_log_level                                          ( enum )   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class console_log_level { kDebug, kInfo, kWarning, kError, kCritical };

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] console_log_color                                          ( enum )   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class console_log_color {
  kDefault,
  kBlack,
  kRed,
  kGreen,
  kYellow,
  kBlue,
  kMagenta,
  kCyan,
  kWhite
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] console_logger_options                                     ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct console_logger_options {
  bool show_name{true};
  bool show_timestamp{true};
  bool show_function{true};
  bool show_line{true};
};
}  // namespace martianlabs::doba::common

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] PLATFORM-DEPENDENT-INCLUDEs                               ( section ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include "common/console_logger_windows.h"
#elif __linux__
#include "common/console_logger_linux.h"
#endif

#endif
