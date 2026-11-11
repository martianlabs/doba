<a name="custom-transport"></a>
<h1>
  <a href="../../../resources/examples/transport/custom_transport/h1-custom-transport.svg#gh-light-mode-only">
    <img src="../../../resources/examples/transport/custom_transport/h1-custom-transport.svg" alt="Custom transport">
  </a>
  <a href="../../../resources/examples/transport/custom_transport/h1-custom-transport-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/transport/custom_transport/h1-custom-transport-dark.svg" alt="Custom transport">
  </a>
</h1>

This example implements an in-memory transport for doba's HTTP/1.1 server. It
creates one protocol engine, passes it a fixed `GET /hello` request, and copies
the response into a string. The executable prints the HTTP response and exits.
No network listener is opened.

`http_server` owns the router and provides the engine factory. Construction
stores the transport configuration; `start()` creates the transport, which
creates its engine and handles the request. When the engine requests closure,
the transport releases that connection. The application then calls
`server.stop()` to end the server lifecycle.

Build and run the `custom_transport` target. The output starts with
`HTTP/1.1 200` and ends with the body `hello from memory`.

The transport shows where incoming and outgoing bytes cross the protocol
boundary. It processes one fixed request synchronously in memory.
It does not demonstrate socket concurrency, TCP policies, timers or application
execution. The native [TCP contract](../../../docs/reference/transport/tcp.md#runtime-contract)
applies to the bundled network backends; the engine/transport concepts check
signatures and do not make this demo a production network implementation.

The engine contract returns processing_result from both input and processing
entry points. This synchronous example returns the empty result and requests
no additional processing. Input bytes remain borrowed only for the call.
Native TCP and TLS support continuation turns. This minimal in-memory
transport deliberately implements only the empty result and rejects nonempty
requests; it does not demonstrate a general native scheduler.
