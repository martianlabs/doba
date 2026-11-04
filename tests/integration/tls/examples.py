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

import contextlib
import os
import socket
import ssl
import subprocess
import sys
import time

from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'helpers'))
from sanitizer_diagnostics import require_clean_diagnostics

if not __debug__:
  raise RuntimeError('TLS example tests require assertions enabled')

if not ssl.HAS_TLSv1_3:
  raise RuntimeError('TLS tests require a TLS 1.3 capable Python ssl module')

HTTPS, ECHO, CERT, KEY = sys.argv[1:5]
CONTEXT = ssl.create_default_context(cafile=CERT)


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] running                                                  ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
@contextlib.contextmanager
def running(binary):
  with socket.socket() as probe:
    probe.bind(('127.0.0.1', 0))
    port = probe.getsockname()[1]
  options = {}
  if os.name == 'nt':
    # The console signal example needs a console; keep its private one hidden.
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    options = dict(startupinfo=startup,
                   creationflags=subprocess.CREATE_NEW_CONSOLE)
  child = subprocess.Popen([binary, CERT, KEY, str(port)],
      stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
      **options)
  try:
    deadline = time.monotonic() + 10
    while True:
      if child.poll() is not None:
        raise AssertionError('Example exited: ' + str(child.communicate()))
      try:
        peer = CONTEXT.wrap_socket(socket.create_connection(
            ('127.0.0.1', port), timeout=1), server_hostname='localhost')
        peer.close()
        break
      except (ConnectionError, TimeoutError):
        if time.monotonic() > deadline:
          raise
        time.sleep(.02)
    yield port
    assert child.poll() is None, 'Example terminated before fixture shutdown'
  finally:
    if child.poll() is None:
      child.terminate()
    try:
      stdout, stderr = child.communicate(timeout=5)
    except subprocess.TimeoutExpired:
      child.kill()
      stdout, stderr = child.communicate(timeout=5)
      raise AssertionError('TLS example did not terminate')
    require_clean_diagnostics(stderr)


# /////////////////////////////////////////////////////////////////////////////
# +---------------------------------------------------------------------------+
# | [>] main                                                     ( function ) |
# +---------------------------------------------------------------------------+
# /////////////////////////////////////////////////////////////////////////////
def main():
  CONTEXT.set_alpn_protocols(['http/1.1'])
  with running(HTTPS) as port:
    with CONTEXT.wrap_socket(socket.create_connection(
        ('127.0.0.1', port), timeout=5), server_hostname='localhost') as peer:
      assert peer.selected_alpn_protocol() == 'http/1.1'
      for path, expected in (('/hello', b'hello over TLS'),
                             ('/wait/7', b'7')):
        peer.sendall(f'GET {path} HTTP/1.1\r\nHost: localhost\r\n\r\n'.encode())
        import http.client
        with http.client.HTTPResponse(peer) as response:
          response.begin()
          assert response.status == 200
          assert response.read() == expected
  CONTEXT.set_alpn_protocols([])
  with running(ECHO) as port:
    with CONTEXT.wrap_socket(socket.create_connection(
        ('127.0.0.1', port), timeout=5), server_hostname='localhost') as peer:
      peer.sendall(b'hel')
      peer.sendall(b'lo\n')
      result = b''
      while len(result) < len(b'echo: hello\n'):
        data = peer.recv(4096)
        assert data
        result += data
      assert result == b'echo: hello\n'
      peer.sendall(b'x' * 4097 + b'\n')
      assert peer.recv(1) == b''
  print('HTTPS and custom TLS executable examples: PASS')


if __name__ == '__main__':
  main()
