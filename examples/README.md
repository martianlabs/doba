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

Use `http_server<>` for HTTP/1.1 over TCP. For TLS, use `https_server<>` as in
[https_hello_world](http/v11/https_hello_world/README.md). To change HTTP's
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

