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

import subprocess
import sys

sys.dont_write_bytecode = True
from sanitizer_diagnostics import require_clean_diagnostics


# +---------------------------------------------------------------------------+
# | [>] main                                                     ( function ) |
# +---------------------------------------------------------------------------+
def main():
  require_clean_diagnostics('ordinary example diagnostics\n')
  require_clean_diagnostics('AddressSanitizer Init done\n')
  findings = [
      'ERROR: AddressSanitizer: heap-use-after-free',
      'WARNING: ThreadSanitizer: data race',
      'SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior',
      'ERROR: LeakSanitizer: detected memory leaks',
      'FATAL: ThreadSanitizer: unexpected memory mapping',
      'AddressSanitizer:DEADLYSIGNAL',
      'test.cpp:2: runtime error: index out of bounds',
  ]
  for finding in findings:
    child = subprocess.run(
        [sys.executable, '-c',
         'import sys; sys.stderr.write(sys.argv[1])', finding],
        capture_output=True, timeout=5)
    if child.returncode != 0:
      raise AssertionError('Negative diagnostic fixture did not exit cleanly')
    try:
      require_clean_diagnostics(child.stderr)
    except AssertionError:
      continue
    raise AssertionError('Silent child diagnostic was accepted: ' + finding)
  print('Child sanitizer diagnostics, including exit zero: PASS')


if __name__ == '__main__':
  main()
