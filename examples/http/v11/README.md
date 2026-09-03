# HTTP/1.1 examples

Build the project from the repository root as described in the
[examples index](../../README.md). Every HTTP server listens on
`localhost:8080`; build the target named by its directory, run that executable,
one server at a time, and stop it with Ctrl+C.

| Area | Examples |
| --- | --- |
| Routing | `hello_world`, `static_routes`, `parametrized_routes`, `wildcard_routes` |
| Request data | `request_information`, `query_parameters`, `request_headers`, `cookies` |
| Request bodies | `request_body_text`, `request_body_binary`, `expect_continue` |
| Response construction | `response_statuses`, `response_headers`, `response_body_values`, `response_body_writers`, `large_response_bodies` |
| Server behavior | `head_requests`, `automatic_not_found`, `method_not_allowed`, `options_asterisk`, `connection_close`, `automatic_date`, `request_rejections` |
| Asynchrony | `asynchronous_routes` |

Each example README contains a `curl` command and its observable result.

