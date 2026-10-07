# Asynchronous routes

This example registers a coroutine route and a synchronous route on the same
HTTP/1.1 server. `GET /slow` yields to its transport worker, sleeps for about
50 ms, and returns `slow`. `GET /fast` returns `fast` synchronously.

```text
curl -i http://localhost:8080/slow
curl -i http://localhost:8080/fast
```

To check response order on one connection, run this while the example is
listening:

```python
import socket

with socket.create_connection(("localhost", 8080), timeout=3) as connection:
    connection.sendall(
        b"GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"
        b"GET /fast HTTP/1.1\r\nHost: localhost\r\n"
        b"Connection: close\r\n\r\n"
    )
    wire = bytearray()
    while chunk := connection.recv(4096):
        wire.extend(chunk)

assert wire.count(b"HTTP/1.1 200 OK") == 2
assert b"\r\n\r\nslow" in wire
assert b"\r\n\r\nfast" in wire
assert wire.index(b"\r\n\r\nslow") < wire.index(b"\r\n\r\nfast")
print("slow, fast")
```

The responses keep request order even though `/fast` finishes first.
`server.stop()` waits for pending coroutines before stopping the workers.
