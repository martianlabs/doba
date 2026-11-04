#                              _       _
#                           __| | ___ | |__   __ _
#                          / _` |/ _ \| '_ \ / _` |
#                         | (_| | (_) | |_) | (_| |
#                          \__,_|\___/|_.__/ \__,_|
#
#                              Apache License
#                        Version 2.0, January 2004
#                     http://www.apache.org/licenses/LICENSE-2.0
#
# Copyright 2026 martianLabs
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

import re


# +---------------------------------------------------------------------------+
# | [>] require_clean_diagnostics                                ( function ) |
# +---------------------------------------------------------------------------+
def require_clean_diagnostics(stderr):
  if isinstance(stderr, bytes):
    stderr = stderr.decode('utf-8', errors='replace')
  sanitizer = (r'(?:Address|Leak|Memory|Thread|UndefinedBehavior)Sanitizer')
  report = (r'(?:ERROR|WARNING|SUMMARY|FATAL):\s*' + sanitizer
            + r'|' + sanitizer + r':\s*(?:DEADLYSIGNAL|reported \d+ warnings)'
            + r'|runtime error:')
  if re.search(report, stderr):
    raise AssertionError('Sanitizer diagnostic in child process:\n' + stderr)
