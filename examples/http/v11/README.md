# HTTP/1.1 examples

Build the project from the repository root as described in the
[examples index](../../README.md). The TCP examples listen on
`0.0.0.0:8080`; the TLS example listens on `0.0.0.0:8443`. Build the target
named by its directory, run that executable, one server at a time, and stop it
with Ctrl+C.

| Area | Examples |
| --- | --- |
| Routing | `hello_world`, `static_routes`, `parametrized_routes`, `wildcard_routes` |
| Request data | `request_information`, `query_parameters`, `request_headers`, `cookies` |
| Request body | `request_body_text`, `request_body_binary`, `request_limits` |
| Response construction | `response_statuses`, `response_headers`, `response_body_values`, `response_body_writers`, `large_response_bodies` |
| Server behavior | `head_requests` |
| Controllers | `controllers` |
| Static files | `static_file_server`, `file_download` |
| TLS | `https_hello_world` |

Each example README contains a `curl` command and its observable result.

The transport is configured when constructing the server:

```cpp
server<> http_server({.ip = "0.0.0.0", .port = "8080"});
http_server.start();
```

Engine policies are optional and follow the transport policies. The
[request limits example](request_limits/README.md) configures one explicitly.
The [TLS example](https_hello_world/README.md) uses `DOBA_ENABLE_TLS=ON` and
requires an OpenSSL certificate and private key.
