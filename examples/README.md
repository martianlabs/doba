# Examples

Start with [hello_world](http/v11/hello_world/README.md), then try
[static_routes](http/v11/static_routes/README.md),
[request_body_text](http/v11/request_body_text/README.md), and
[response_body_values](http/v11/response_body_values/README.md). The
[HTTP/1.1 examples](http/v11/README.md) continue from there by topic.

Configure and build the project from the repository root before running an
executable:

```text
cmake -S . -B out/build/release -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/release
```

Use `http_server<>` for HTTP/1.1 over TCP. Optional TLS on Linux and Windows is demonstrated by
[HTTPS](http/v11/https_hello_world/README.md) and
[a custom TLS protocol](protocol/tls_echo/README.md). To change HTTP's
request, response, or router, follow the
[custom request](http/v11/custom_http_request/README.md),
[custom response](http/v11/custom_http_response/README.md), and
[custom router](http/v11/custom_http_router/README.md) examples.

The generic `server<...>` joins an engine and a transport without HTTP routing.
The [custom protocol](protocol/custom_protocol/README.md) example pairs it
with TCP. The [custom transport](transport/server/custom_transport/README.md)
example keeps HTTP/1.1 and runs it in memory. Explore these after the HTTP
examples if you need to replace either side.

[Common utilities](common/README.md) has standalone examples to consult when
you need their APIs.


Network examples use TCP or the separate TCP-based TLS class on
Linux and Windows. Neither transport supplies an application executor. The
[processing-turn example](http/v11/processing_turns/README.md) demonstrates
concurrent handlers through the single HTTP engine using an in-memory adapter.
Linux and Windows TCP supply these turns natively; see
[async_routes](http/v11/async_routes/README.md). TLS forwards them too; the
[HTTPS example](http/v11/https_hello_world/README.md) includes an async route.
A separate executor is not part of the engine interface.

See the [TCP runtime contract](../docs/reference/transport/server/tcp.md#runtime-contract)
and [policy reference](../docs/reference/transport/server/tcp_policies.md) for
callback lifetime, a terminal prefix/source reader, hard limits and stop behavior.
