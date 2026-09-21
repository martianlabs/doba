# HTTP/1.1 examples

Build the project from the repository root as described in the
[examples index](../../README.md). Every HTTP server listens on
`0.0.0.0:8080` (accessible through `localhost:8080`); build the target named by its directory, run that executable,
one server at a time, and stop it with Ctrl+C.

| Area | Examples |
| --- | --- |
| Routing | `hello_world`, `static_routes`, `parametrized_routes`, `wildcard_routes` |
| Request data | `request_information`, `query_parameters`, `request_headers`, `cookies` |
| Request bodies | `request_body_text`, `request_body_binary`, `expect_continue` |
| Response construction | `response_statuses`, `response_headers`, `response_body_values`, `response_body_writers`, `large_response_bodies` |
| Server behavior | `head_requests`, `automatic_not_found`, `method_not_allowed`, `options_asterisk`, `connection_close`, `automatic_date`, `request_rejections` |
| Asynchrony | `asynchronous_routes` |
| Controllers | `controllers` |
| Static files | `static_file_server` |

Each example README contains a `curl` command and its observable result.

The transport is configured when constructing the server:

```cpp
server<> http_server({.ip = "0.0.0.0", .port = "8080"});
http_server.start();
```

Engine policies are optional and follow the transport policies. The current
engine migration still lacks asynchronous handler execution, interim
`100 Continue` responses, and HTTP responses for decoder rejections. The
corresponding examples build and run, but their intended results remain pending.
