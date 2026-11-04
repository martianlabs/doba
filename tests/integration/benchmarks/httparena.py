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

import concurrent.futures
import contextlib
import json
import os
import random
import socket
import subprocess
import sys
import threading
import time

from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'helpers'))
from sanitizer_diagnostics import require_clean_diagnostics

if not __debug__:
  raise RuntimeError('HttpArena checks require Python assertions enabled')

BINARY, METADATA = sys.argv[1:3]
RANDOM = random.SystemRandom()


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] running                                                  ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
@contextlib.contextmanager
def running():
  with socket.socket() as probe:
    probe.bind(('127.0.0.1', 0))
    port = probe.getsockname()[1]
  env = dict(os.environ, WORKERS='1', PORT=str(port))
  child = subprocess.Popen([BINARY], env=env, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE)
  try:
    deadline = time.perf_counter() + 10
    while True:
      if child.poll() is not None:
        raise AssertionError('Driver exited: ' + str(child.communicate()))
      try:
        with socket.create_connection(('127.0.0.1', port), timeout=.2):
          break
      except OSError:
        assert time.perf_counter() < deadline, 'Driver startup timeout'
        time.sleep(.02)
    yield port
    assert child.poll() is None, 'Driver exited during validation'
  finally:
    if child.poll() is None:
      child.terminate()
    try:
      _, errors = child.communicate(timeout=5)
      require_clean_diagnostics(errors)
    except subprocess.TimeoutExpired:
      child.kill()
      child.communicate(timeout=5)
      raise AssertionError('Driver process did not terminate')


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] response                                                 ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def response(stream):
  status = stream.readline()
  assert status.startswith(b'HTTP/1.1 '), status
  headers = {}
  while True:
    line = stream.readline()
    assert line, 'Unexpected response EOF'
    if line == b'\r\n':
      break
    name, value = line.split(b':', 1)
    name = name.lower()
    assert name not in headers, ('duplicate header', name)
    headers[name] = value.strip()
  assert b'content-length' in headers and b'transfer-encoding' not in headers
  length = int(headers[b'content-length'])
  assert 0 <= length <= 1024 * 1024
  body = stream.read(length)
  assert len(body) == length
  return int(status.split()[1]), headers, body


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] request                                                  ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def request(path, body=None, chunked=False):
  method = 'GET' if body is None else 'POST'
  head = f'{method} {path} HTTP/1.1\r\nHost: local\r\n'
  if body is None:
    return (head + '\r\n').encode()
  raw = str(body).encode()
  if chunked:
    return ((head + 'Transfer-Encoding: chunked\r\n\r\n').encode() +
            f'{len(raw):x}\r\n'.encode() + raw + b'\r\n0\r\n\r\n')
  return ((head + f'Content-Length: {len(raw)}\r\n\r\n').encode() + raw)


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] delay                                                    ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def delay(port, milliseconds, barrier=None):
  with socket.create_connection(('127.0.0.1', port), timeout=15) as peer:
    with peer.makefile('rb') as stream:
      if barrier:
        barrier.wait(timeout=10)
      # Python <=3.12 on Windows may give monotonic() a 15.6 ms tick.
      # Use the monotonic high-resolution counter; do not relax the deadline.
      start = time.perf_counter_ns()
      peer.sendall(request(f'/delay/{milliseconds}'))
      status, headers, body = response(stream)
      elapsed = time.perf_counter_ns() - start
      assert status == 200 and body == str(milliseconds).encode(), body
      assert headers[b'content-type'] == b'text/plain', headers
      assert elapsed >= milliseconds * 1000000, (milliseconds, elapsed)
      return elapsed / 1000000000


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] verify                                                   ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def verify(port):
  with socket.create_connection(('127.0.0.1', port), timeout=5) as peer:
    with peer.makefile('rb') as stream:
      for mode in range(3):
        for _ in range(9):
          a, b, c = (RANDOM.randint(-100000, 100000) for _ in range(3))
          raw = request(f'/baseline11?a={a}&b={b}',
                        None if mode == 0 else c, mode == 2)
          peer.sendall(raw)
          status, _, body = response(stream)
          expected = a + b + (c if mode else 0)
          assert (status, body) == (200, str(expected).encode()), expected
      # Fragments must be owned by the engine until a full request is decoded.
      for byte in request('/baseline11?a=17&b=29', 31, True):
        peer.sendall(bytes([byte]))
      assert response(stream)[::2] == (200, b'77')
      peer.sendall(request('/pipeline') * 16)
      for _ in range(16):
        status, headers, body = response(stream)
        assert (status, body) == (200, b'ok')
        assert headers[b'content-type'] == b'text/plain'
      for raw in (request('/baseline11?a=9223372036854775807&b=1'),
                  request('/baseline11?a=-9223372036854775808&b=-1'),
                  request('/baseline11?a=9223372036854775807&b=0', 1),
                  request('/baseline11?a=1&b=2', 'not-an-integer'),
                  request('/delay/-1'), request('/delay/60001'),
                  request('/delay/not-a-number')):
        peer.sendall(raw)
        assert response(stream)[0] == 400
      # Same-connection mixed requests must emit strictly in request order.
      mixed = ('/delay/90', '/baseline11?a=11&b=23', '/delay/10', '/pipeline')
      start = time.perf_counter_ns()
      peer.sendall(b''.join(request(path) for path in mixed))
      for expected in (b'90', b'34', b'10', b'ok'):
        status, _, body = response(stream)
        assert (status, body) == (200, expected), (expected, body)
      assert time.perf_counter_ns() - start >= 90000000
  assert delay(port, 0) >= 0
  for value in RANDOM.sample(range(10, 91), 4):
    delay(port, value)
  # Min of three short probes tolerates a single scheduling pause. No batch
  # throughput/latency ceiling is asserted; this rejects a fixed long sleep.
  short = min(delay(port, 10) for _ in range(3))
  long = delay(port, 500)
  assert long - short >= .3, (short, long)
  barrier = threading.Barrier(33)
  values = RANDOM.sample(range(100, 500), 32)
  start = time.perf_counter()
  with concurrent.futures.ThreadPoolExecutor(max_workers=32) as pool:
    futures = [pool.submit(delay, port, value, barrier) for value in values]
    barrier.wait(timeout=10)
    for future in futures:
      future.result(timeout=20)
  print(f'32 independent dynamic delays: {time.perf_counter() - start:.3f}s')


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] main                                                  ( entry-point ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def main():
  clock = time.get_clock_info('perf_counter')
  assert clock.monotonic and clock.resolution <= .0001, clock
  with open(METADATA, encoding='utf-8') as source:
    assert json.load(source)['tests'] == ['baseline', 'pipelined', 'async']
  for key, value in (('WORKERS', '0'), ('WORKERS', '-1'), ('WORKERS', 'x'),
                     ('PORT', '0'), ('PORT', '65536'), ('PORT', 'x')):
    env = dict(os.environ, WORKERS='1', PORT='8080')
    env[key] = value
    result = subprocess.run([BINARY], env=env, capture_output=True, timeout=5)
    assert result.returncode == 1 and key.encode() in result.stderr, (
        key, value, result.returncode, result.stderr.decode(errors='replace'))
  with running() as port:
    verify(port)
  print('HttpArena baseline, pipelined and dynamic async correctness: PASS')


if __name__ == '__main__':
  main()
