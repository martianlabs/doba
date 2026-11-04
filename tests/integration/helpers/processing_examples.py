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
# Copyright 2025 martianLabs
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

import socket
import subprocess
import sys
import time

from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'helpers'))
from sanitizer_diagnostics import require_clean_diagnostics

if not __debug__:
  raise RuntimeError('Processing examples require assertions enabled')

TASK, MEMORY, NETWORK = sys.argv[1:4]


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] main                                                  ( entry-point ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def main():
  for binary, expected in ((TASK, 'Completed steps: 3'),
      (MEMORY, 'Concurrent handlers: 3; ordered output: 1 2 3')):
    result = subprocess.run([binary], capture_output=True, text=True,
                            timeout=10)
    require_clean_diagnostics(result.stderr)
    assert result.returncode == 0, (result.returncode, result.stderr)
    assert result.stdout.strip() == expected, result.stdout
  with socket.socket() as probe:
    probe.bind(('127.0.0.1', 0))
    port = probe.getsockname()[1]
  child = subprocess.Popen([NETWORK, str(port)], stdin=subprocess.PIPE,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  try:
    end = time.monotonic() + 10
    while True:
      assert child.poll() is None, 'TCP example exited during startup'
      try:
        peer = socket.create_connection(('127.0.0.1', port), timeout=.2)
        break
      except OSError:
        assert time.monotonic() < end
        time.sleep(.02)
    with peer:
      peer.settimeout(5)
      paths = ('/wait/1', '/sync', '/wait/2')
      peer.sendall(''.join(f'GET {p} HTTP/1.1\r\nHost: local\r\n\r\n'
                           for p in paths).encode())
      with peer.makefile('rb') as stream:
        for expected in (b'1', b'sync', b'2'):
          assert stream.readline().startswith(b'HTTP/1.1 200 ')
          headers = {}
          while True:
            line = stream.readline()
            assert line
            if line == b'\r\n':
              break
            name, value = line.split(b':', 1)
            assert name.lower() not in headers
            headers[name.lower()] = value.strip()
          size = int(headers[b'content-length'])
          assert 0 <= size <= 1024 * 1024
          assert stream.read(size) == expected
    stdout, stderr = child.communicate(input=b'\n', timeout=5)
    require_clean_diagnostics(stderr)
    assert child.returncode == 0, (child.returncode, stdout, stderr)
  finally:
    if child.poll() is None:
      child.kill()
      child.communicate(timeout=5)
  print('Managed task, in-memory engine and native TCP examples: PASS')


if __name__ == '__main__':
  main()
