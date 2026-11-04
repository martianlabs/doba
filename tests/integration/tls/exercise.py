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
import socket
import ssl
import subprocess
import sys
import threading
import time

from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'helpers'))
from sanitizer_diagnostics import require_clean_diagnostics

if not __debug__:
  raise RuntimeError('TLS tests require Python assertions enabled')

if not ssl.HAS_TLSv1_3:
  raise RuntimeError('TLS tests require a TLS 1.3 capable Python ssl module')

BINARY, CERT, KEY = sys.argv[1:4]


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] server                                                   ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
@contextlib.contextmanager
def server(mode='echo', plain=1048576, bio=17408, cipher=2097152,
           control=False):
  with socket.socket() as probe:
    probe.bind(('127.0.0.1', 0))
    port = probe.getsockname()[1]
  process = subprocess.Popen([BINARY, CERT, KEY, str(port), mode, str(plain),
                              str(bio), str(cipher)], stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             text=True)
  ready = []
  waiter = threading.Thread(
      target=lambda: ready.append(process.stdout.readline()))
  waiter.start()
  try:
    waiter.join(10)
    if waiter.is_alive() or ready != ['READY\n']:
      raise AssertionError('TLS fixture did not start: ' + str(ready))
    yield (port, process) if control else port
  finally:
    if process.poll() is None:
      process.stdin.write('stop\n')
      process.stdin.flush()
    try:
      stdout, stderr = process.communicate(timeout=10)
    except subprocess.TimeoutExpired:
      process.kill()
      stdout, stderr = process.communicate()
      raise AssertionError('TLS fixture failed to stop: ' + stderr)
    waiter.join(1)
    require_clean_diagnostics(stderr)
    if process.returncode != 0:
      raise AssertionError(f'TLS fixture exit {process.returncode}: {stderr}')


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] connect                                                  ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def connect(port, version=None, alpn='doba-echo'):
  context = ssl.create_default_context(cafile=CERT)
  if version:
    context.minimum_version = context.maximum_version = version
  if alpn is not None:
    context.set_alpn_protocols([alpn])
  raw = socket.create_connection(('127.0.0.1', port), timeout=5)
  try:
    return context.wrap_socket(raw, server_hostname='localhost')
  except BaseException:
    raw.close()
    raise


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] receive                                                  ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def receive(peer, expected):
  result = b''
  while len(result) < len(expected):
    block = peer.recv(min(65536, len(expected) - len(result)))
    if not block:
      break
    result += block
  assert result == expected, (len(result), len(expected), result[:80])


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] fragmented                                               ( function ) |
# +---------------------------------------------------------------------------+
# | Drive a verified TLS client with one-byte encrypted socket writes.        |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def fragmented(port):
  context = ssl.create_default_context(cafile=CERT)
  context.set_alpn_protocols(['doba-echo'])
  incoming, outgoing = ssl.MemoryBIO(), ssl.MemoryBIO()
  client = context.wrap_bio(incoming, outgoing, server_hostname='localhost')
  with socket.create_connection(('127.0.0.1', port), timeout=5) as raw:
    def drain():
      data = outgoing.read()
      for byte in data:
        raw.sendall(bytes([byte]))
    while True:
      try:
        client.do_handshake()
        drain()
        break
      except ssl.SSLWantReadError:
        drain()
        data = raw.recv(65536)
        assert data, "Unexpected TLS EOF"
        incoming.write(data)
    client.write(b'fragmented\n')
    drain()
    result = b''
    while len(result) < len(b'echo:fragmented\n'):
      try:
        result += client.read(65536)
      except ssl.SSLWantReadError:
        drain()
        data = raw.recv(65536)
        assert data, "Unexpected TLS EOF"
        incoming.write(data)
    assert result == b'echo:fragmented\n'


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] main                                                     ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def main():
  with server() as port:
    for version in (ssl.TLSVersion.TLSv1_2, ssl.TLSVersion.TLSv1_3):
      with connect(port, version) as peer:
        assert peer.selected_alpn_protocol() == 'doba-echo'
        peer.sendall(b'a\nb\n' * 16)
        receive(peer, b'echo:a\necho:b\n' * 16)
        peer.sendall(b'source\n')
        receive(peer, b'pre:body:tail')
        peer.sendall(b'empty\n')
        receive(peer, b'pre:body:')
        peer.sendall(b'large\n')
        receive(peer, b'pre:' + b's' * (128 * 1024))
        peer.sendall(b'source-close\n')
        receive(peer, b'pre:body:tail')
        assert peer.recv(1) == b''
        with peer.unwrap():
          pass
    with connect(port) as peer:
      peer.sendall(b'x' * 33000 + b'\n')
      receive(peer, b'echo:' + b'x' * 33000 + b'\n')
      peer.sendall(b'released\n')
      receive(peer, b'tail')
      peer.sendall(b'after\n')
      receive(peer, b'echo:after\n')
      with peer.unwrap():
        pass
    with connect(port) as peer:
      peer.sendall(b'empty-close\n')
      receive(peer, b'pre:body:')
      assert peer.recv(1) == b''
      with peer.unwrap():
        pass
    with connect(port) as peer:
      peer.sendall(b'second-source\n')
      assert peer.recv(1) == b''
    fragmented(port)
    # A verified client must reject unknown credentials and the wrong name.
    for context, hostname in ((ssl.create_default_context(), 'localhost'),
        (ssl.create_default_context(cafile=CERT), 'wrong.invalid')):
      with socket.create_connection(('127.0.0.1', port), timeout=5) as raw:
        try:
          context.wrap_socket(raw, server_hostname=hostname)
          raise AssertionError('Untrusted certificate or hostname accepted')
        except ssl.SSLCertVerificationError:
          pass
    # Multiple independent sessions exercise both platform worker models.
    def exchange(index):
      with connect(port) as peer:
        text = str(index).encode()
        peer.sendall(text + b'\n')
        receive(peer, b'echo:' + text + b'\n')
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
      list(pool.map(exchange, range(32)))
    for token in ('h2', 'unrelated'):
      try:
        connect(port, alpn=token)
        raise AssertionError('ALPN mismatch accepted')
      except ssl.SSLError:
        pass
    with socket.create_connection(('127.0.0.1', port), timeout=5) as bad:
      bad.sendall(b'not TLS\n')
      assert bad.recv(1) == b''
    # An invalid engine batch is discarded atomically.
    with connect(port) as bad:
      bad.sendall(b'bad\n')
      assert bad.recv(1) == b''
    with connect(port) as bad:
      bad.sendall(b'failed\n')
      assert bad.recv(1) == b''
    exchange(99)
    # Abrupt EOF during handshake or application traffic must not poison reuse.
    with socket.create_connection(('127.0.0.1', port), timeout=5) as truncated:
      truncated.sendall(b'\x16\x03')
    exchange(100)
    # Incomplete unauthenticated handshakes cannot occupy the sole worker.
    with contextlib.ExitStack() as pending:
      for _ in range(4):
        stalled = pending.enter_context(socket.create_connection(
            ('127.0.0.1', port), timeout=5))
        stalled.sendall(b'\x16\x03\x03\x40\x00\x01')
      exchange(101)
    exchange(102)
  print('TLS 1.2/1.3, pipeline, source, close, fragmentation '
        'and isolation: PASS')
  with server(plain=4096, bio=127) as port:
    with connect(port) as peer:
      peer.sendall(b'exact\n')
      receive(peer, b'e' * 4096)
    with connect(port) as peer:
      peer.sendall(b'over\n')
      assert peer.recv(1) == b''
    with connect(port) as peer:
      peer.sendall(b'ok\n')
      receive(peer, b'echo:ok\n')
    with connect(port) as peer:
      peer.sendall(b'inline-exact\n')
      receive(peer, b'pre:' + b'e' * 4092)
    for command in (b'inline-over\n', b'sum-over\n'):
      with connect(port) as peer:
        peer.sendall(command)
        assert peer.recv(1) == b''
  print('Tiny BIO, inline/source budgets and cumulative overflow: PASS')
  with server(cipher=65536) as port:
    with connect(port) as peer:
      peer.sendall(b'large\n')
      assert peer.recv(1) == b''
    with connect(port) as peer:
      peer.sendall(b'ok\n')
      receive(peer, b'echo:ok\n')
  with server(mode='admission', control=True) as (port, process):
    with socket.create_connection(('127.0.0.1', port), timeout=5) as first:
      first.sendall(b'\x16\x03')
      admitted = []
      waiter = threading.Thread(
          target=lambda: admitted.append(process.stdout.readline()))
      waiter.start()
      waiter.join(10)
      assert admitted == ['ADMITTED\n']
      try:
        connect(port)
        raise AssertionError('TLS admission ceiling exceeded')
      except (ssl.SSLError, ConnectionError):
        pass
    deadline = time.monotonic() + 5
    while True:
      try:
        recovered = connect(port)
        break
      except (ssl.SSLError, ConnectionError):
        if time.monotonic() > deadline:
          raise
        time.sleep(.01)
    with recovered:
      recovered.sendall(b'recovered\n')
      receive(recovered, b'echo:recovered\n')
  with server(control=True) as (port, process):
    with connect(port) as peer:
      peer.sendall(b'before\n')
      receive(peer, b'echo:before\n')
    process.stdin.write('restart\n')
    process.stdin.flush()
    ready = []
    worker = threading.Thread(
        target=lambda: ready.append(process.stdout.readline()))
    worker.start()
    worker.join(10)
    assert ready == ['READY\n']
    with connect(port) as peer:
      peer.sendall(b'after\n')
      receive(peer, b'echo:after\n')
  with server(plain=5 * 1024 * 1024, cipher=6 * 1024 * 1024) as port:
    with connect(port) as peer:
      peer.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)
      peer.sendall(b'slow\n')
      time.sleep(.1)
      receive(peer, b'pre:' + b's' * (4 * 1024 * 1024))
      peer.sendall(b'after\n')
      receive(peer, b'echo:after\n')
    # Stop or a reset during queued ciphertext must release every context.
    with connect(port) as peer:
      peer.sendall(b'slow\n')
  print('Large bounded source, slow peer and pending-output teardown: PASS')
  with server(cipher=1) as port:
    try:
      connect(port)
      raise AssertionError('Handshake exceeded ciphertext budget')
    except (ssl.SSLError, ConnectionError):
      pass
  with server(mode='http') as port:
    with connect(port, alpn='http/1.1') as peer:
      peer.sendall(b'GET /hello HTTP/1.1\r\nHost: localhost\r\n\r\n' * 16)
      data = b''
      while data.count(b'\r\n\r\nhello') < 16:
        block = peer.recv(65536)
        assert block
        data += block
      assert data.count(b'HTTP/1.1 200') == 16
      assert data.count(b'\r\n\r\nhello') == 16
    with connect(port, alpn=None) as peer:
      assert peer.selected_alpn_protocol() is None
      peer.sendall(b'GET /hello HTTP/1.1\r\nHost: localhost\r\n\r\n')
      import http.client
      with http.client.HTTPResponse(peer) as response:
        response.begin()
        assert response.status == 200 and response.read() == b'hello'
  print('HTTPS pipeline and clients without ALPN: PASS')
  with server(mode='http-auth') as port:
    with connect(port, alpn='http/1.1') as peer:
      def request(authorization=None):
        import http.client
        head = b'GET /private HTTP/1.1\r\nHost: localhost\r\n'
        if authorization:
          head += b'Authorization: Basic ' + authorization + b'\r\n'
        peer.sendall(head + b'\r\n')
        with http.client.HTTPResponse(peer) as response:
          response.begin()
          return response.status, response.read()
      assert peer.selected_alpn_protocol() == 'http/1.1'
      assert request()[0] == 401
      assert request(b'YWxpY2U6c2VjcmV0') == (200, b'private')
  # Configuration errors must occur before a listening socket is created.
  for plain, bio, cipher in ((0, 1, 1), (1, 0, 1), (1, 1, 0)):
    result = subprocess.run([BINARY, CERT, KEY, '8443', 'echo', str(plain),
                             str(bio), str(cipher)], capture_output=True,
                            timeout=5)
    assert result.returncode == 1
    assert b'Invalid TLS policies' in result.stderr
  result = subprocess.run([BINARY, CERT + '.missing', KEY, '8443', 'echo'],
                          capture_output=True, timeout=5)
  assert result.returncode == 1
  assert b'Cannot configure TLS certificate or key' in result.stderr
  result = subprocess.run([BINARY, CERT, KEY, '8443', 'http-invalid'],
                          capture_output=True, timeout=5)
  assert result.returncode == 1 and b'ALPN' in result.stderr
  print('Invalid policies and credentials rejected before listening: PASS')


if __name__ == '__main__':
  main()
