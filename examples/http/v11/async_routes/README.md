# Cooperative HTTP handlers on TCP

Linux and Windows. Build `http_async_routes` with examples enabled, run it and press
Enter to stop. It listens on `127.0.0.1:8080` with one transport worker.
An optional first argument selects another port, for example
`http_async_routes 18080`. The executable-example test uses an isolated port.

- `GET /wait/:id` waits ten milliseconds and returns the integer route value.
- `GET /sync` returns `sync` through the normal direct handler path.

Several `/wait/:id` requests on one pipelined connection start independent
waits. Responses are emitted in request order; waiting does not occupy the
worker. Other connections can progress during those sleeps. The engine chooses
the handler path automatically; there is no executor injection or mode flag.

[Executable-example checks](../../../../tests/integration/helpers/processing_examples.py)
also run the standalone task owner and in-memory concurrent engine examples.

Only `common::yield()` and `common::sleep_for()` are supported managed awaiters.
A blocking handler still occupies its worker. TCP owns generic connection turn
notifications and socket bytes; the engine owns frames, deadlines and response
ordering. Inline bytes and a terminal reader share the existing output contract.

This example uses plain TCP. For TLS see the coroutine route in
[https_hello_world](../https_hello_world/README.md). The portable
[processing_turns](../processing_turns/README.md) example uses an in-memory
adapter instead. See [the engine contract](../../../../docs/reference/protocol/http/v11/engine.md).
