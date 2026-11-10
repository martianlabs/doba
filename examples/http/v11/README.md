# HTTP/1.1 examples

Build the project from the repository root as described in the
[examples index](../../README.md). The TCP examples listen on
`0.0.0.0:8080`. For optional TLS, see
[https_hello_world](https_hello_world/README.md). Build the target
named by its directory, run that executable, one server at a time, and stop it
with Ctrl+C.

For the shortest path, run [hello_world](hello_world/README.md),
[static_routes](static_routes/README.md),
[request_body_text](request_body_text/README.md), and
[response_body_values](response_body_values/README.md). Then pick the topics
your application needs. Each step below lists examples in reading order:

1. **Start an HTTP server:** [hello_world](hello_world/README.md).
2. **Route requests:** [static_routes](static_routes/README.md),
   [parametrized_routes](parametrized_routes/README.md),
   [wildcard_routes](wildcard_routes/README.md), and
   [controllers](controllers/README.md).
3. **Read request data:**
   [request_information](request_information/README.md),
   [query_parameters](query_parameters/README.md),
   [request_headers](request_headers/README.md), and
   [cookies](cookies/README.md).
4. **Read request bodies and set limits:**
   [request_body_text](request_body_text/README.md),
   [request_body_binary](request_body_binary/README.md), and
   [request_limits](request_limits/README.md).
5. **Build responses:** [response_statuses](response_statuses/README.md),
   [response_headers](response_headers/README.md),
   [response_body_values](response_body_values/README.md),
   [response_body_writers](response_body_writers/README.md), and
   [large_response_bodies](large_response_bodies/README.md).
6. **Serve resources:** [head_requests](head_requests/README.md),
   [static_file_server](static_file_server/README.md), and
   [file_download](file_download/README.md).

Protect individual endpoints with [basic_auth](basic_auth/README.md) or
[bearer_auth](bearer_auth/README.md). Both keep a public health route.

To customize HTTP composition, continue with
[custom_http_request](custom_http_request/README.md),
[custom_http_response](custom_http_response/README.md), and
[custom_http_router](custom_http_router/README.md). For a different protocol
or transport, see the [examples index](../../README.md).

Each example README contains a `curl` command and its observable result.

The transport is configured when constructing the server:

```cpp
http_server<> server({.ip = "0.0.0.0", .port = "8080"});
server.start();
```

Engine policies are optional and follow the transport policies. The
[request limits example](request_limits/README.md) configures one explicitly.
`DOBA_ENABLE_TLS=ON` adds the separate OpenSSL-backed TLS target and example.
Existing TCP examples keep their dependencies and behavior.

The engine supports synchronous and cooperative handlers on both platforms.
Transports supply generic turns; the engine owns coroutine scheduling.
Inline-response pipelining is supported; an
input batch can own only one terminal reader. Later nonempty output or another
reader closes the connection. Empty sends without a reader are no-ops.

See the [runtime contract](../../../docs/reference/transport/tcp.md#runtime-contract)
for ownership and Linux/Windows differences.

[processing_turns](processing_turns/README.md) demonstrates concurrent async
waits and ordered responses on one in-memory connection. It supplies turns
itself; plain Linux/Windows TCP supports native turns in
[async_routes](async_routes/README.md). The
[HTTPS example](https_hello_world/README.md) includes a coroutine route.
