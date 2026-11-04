# Custom protocol

This example implements a line-based protocol engine and runs it on doba's TCP
transport. Each connection has its own engine. Complete lines are returned as
`echo: <line>`. The engine retains an incomplete line between fresh RX blocks;
the transport does not assemble messages. The engine enforces a 4096-byte line
limit including the newline, independently of the transport RX capacity.
Exceeding it requests closure.

The generic `server` combines the engine and TCP transport without HTTP
request, response, or router types. Its constructor stores the transport
policies and engine factory. `start()` creates the transport, which
uses the factory to create an engine for each connection.

Build the `custom_protocol` target, run it, and connect to `localhost:8080`.
For example, send `hello\n` with a TCP client to receive `echo: hello\n`.
Stop the server with Ctrl+C.

Callbacks stay registered in their engine and execute only during a current
serialized input or processing turn. Linux keeps the connection on its owning
worker; Windows can use different IOCP workers between serialized input calls.
The factory and shared application state must support concurrent connections.

Configuration and start/stop belong to a sequential external owner, outside
worker callbacks. Linux administrative stop cancels pending output after the
active call; Windows drains accepted bytes and real I/O. Neither waits for
application tasks. See the [runtime contract](../../../docs/reference/transport/server/tcp.md#runtime-contract).

The engine contract returns processing_result from both input and processing
entry points. This synchronous example returns the empty result and requests
no additional processing. Input bytes remain borrowed only for the call.
Linux and Windows TCP support native continuation turns; TLS forwards them.
This particular example requests none.
