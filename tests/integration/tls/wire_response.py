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

# Final Content-Length responses used by the TLS processing fixture.
# This oracle is independent of doba's production HTTP parser.
# +---------------------------------------------------------------------------+
# | [>] response                                                 ( function ) |
# +---------------------------------------------------------------------------+
def response(stream):
  def require(condition, message):
    if not condition:
      raise AssertionError(message)

  status = stream.readline(16385)
  total = len(status)
  require(status.endswith(b'\r\n') and len(status) >= 15,
          'Invalid response status line')
  require(status.startswith(b'HTTP/1.1 ') and status[12:13] == b' '
          and status[9:12].isdigit(), 'Invalid response status syntax')
  code = int(status[9:12])
  require(200 <= code <= 599, 'Expected a final response status')
  require(all(c >= 32 and c != 127 or c == 9 for c in status[13:-2]),
          'Control byte in response reason')
  headers = {}
  while True:
    line = stream.readline(16385)
    total += len(line)
    require(total <= 16384 and line.endswith(b'\r\n'),
            'Invalid or oversized response head')
    if line == b'\r\n':
      break
    name, separator, value = line[:-2].partition(b':')
    token = b"!#$%&'*+-.^_\x60|~"
    require(separator and name and all(
        65 <= c <= 90 or 97 <= c <= 122 or 48 <= c <= 57 or c in token
        for c in name), 'Invalid response field name')
    require(all(c >= 32 and c != 127 or c == 9 for c in value),
            'Control byte in response field')
    name = name.lower()
    require(name not in (b'content-length', b'transfer-encoding', b'date')
            or name not in headers, 'Duplicate response singleton')
    headers[name] = value.strip(b' \t')
  require(b'content-length' in headers
          and b'transfer-encoding' not in headers, 'Ambiguous response framing')
  length = headers[b'content-length']
  require(length and length.isdigit(), 'Invalid Content-Length')
  size = 0
  for digit in length:
    size = size * 10 + digit - 48
    require(size <= 1024 * 1024, 'Oversized response body')
  body = stream.read(size)
  require(len(body) == size, 'Truncated response body')
  return code, body
