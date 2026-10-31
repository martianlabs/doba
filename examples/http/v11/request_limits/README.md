# Request limits

This example passes HTTP/1.1 `policies` as the second argument to `server`.
It accepts request bodies of at most five bytes. Larger bodies are rejected
before the `POST /upload` handler runs.

The first constructor argument configures TCP: `max_connections = 65535`.
The second configures
HTTP: `max_pending_requests = 128` and `max_pending_response_bytes = 1 MiB`.
These are the defaults; the example spells them out alongside its five-byte
request-body limit. The transport send-byte budget keeps its default **1 MiB**.

Admission applies across one server's workers; HTTP queue limits
apply per connection. TCP enforces hard send limits; an overflow closes the
connection. TLS is outside the current scope.
Zero disables the connection and HTTP pending limits. The single terminal
reader restriction is fixed. The transport send-byte budget
must stay positive. See [limits and operation](../../../../docs/guide/06-limits-and-operation.md#bound-work-in-flight)
for accounting and the boundaries these settings cover.

## Try it

```sh
curl -i --data-binary "12345" http://localhost:8080/upload
curl -i --data-binary "123456" http://localhost:8080/upload
```

The first request returns `200 OK` with `accepted`. The second returns
`413 Content Too Large`.

Linux supports independent SO_REUSEPORT workers; 0 selects one worker.
The new transport supports synchronous callbacks and prefix/source;
TLS and asynchronous routes are outside this phase.

Neither TCP backend provides inactivity expiry or a pressure/resume handshake.
Use max_buffered_send_bytes and max_connections as hard transport limits.
