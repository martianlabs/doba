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

import io
import sys

sys.dont_write_bytecode = True
from wire_response import response


# +---------------------------------------------------------------------------+
# | [>] check_oracle                                             ( function ) |
# +---------------------------------------------------------------------------+
def check_oracle():
  valid = b'HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok'
  stream = io.BytesIO(valid + valid)
  if response(stream) != (200, b'ok') or response(stream) != (200, b'ok'):
    raise AssertionError('Response oracle consumed a successor')
  if stream.read(1):
    raise AssertionError('Response oracle left valid bytes unread')
  binary = (b'HTTP/1.1 201 \r\ncontent-length: \t0002\t\r\n'
            b'X-Info: a\r\nX-Info: b\r\n\r\n\x00\xff')
  if response(io.BytesIO(binary)) != (201, b'\x00\xff'):
    raise AssertionError('Response oracle rejected valid binary framing')
  fields = [
      b'Content-Length: 999\r\ncontent-length: 2',
      b'Content-Length: 2\r\nContent-Length: 2',
      b'Content-Length: 2\r\nTransfer-Encoding: chunked',
      b'Content-Length: +2', b'Content-Length: -1',
      b'Content-Length: 2, 2', b'Content-Length: 1048577',
      b'Content-Length: 2\r\nX Bad: value',
      b'Content-Length: 2\r\nDate: a\r\ndate: b',
      b'Content-Length: 2\r\nX: value\x00hidden',
      b'Content-Length: 2\r\nX: value\x0bhidden',
      b'Content-Length: 2\r\nX: value\x7fhidden',
      b'Content-Length: 2\r\nX: ' + b'x' * 16384,
  ]
  invalid = [b'HTTP/1.1 200 OK\r\n' + field + b'\r\n\r\nok'
             for field in fields]
  invalid += [
      b'HTTP/1.1 099 Invalid\r\nContent-Length: 2\r\n\r\nok',
      b'HTTP/1.1 600 Invalid\r\nContent-Length: 2\r\n\r\nok',
      b'HTTP/1.1 200 bad\x00reason\r\nContent-Length: 2\r\n\r\nok',
      b'HTTP/1.1 200 OK\nContent-Length: 2\n\nok',
      b'HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nok',
      b'HTTP/1.1 200 OK\r\nContent-Length: 2\r\n',
  ]
  for index, wire in enumerate(invalid):
    try:
      response(io.BytesIO(wire))
    except AssertionError:
      continue
    raise AssertionError(f'Response oracle accepted malformed case {index}')


if __name__ == '__main__':
  check_oracle()
  print('Independent TLS response oracle: PASS')
