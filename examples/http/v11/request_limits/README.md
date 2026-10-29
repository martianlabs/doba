# Request limits

This example passes HTTP/1.1 `policies` as the second argument to `server`.
It accepts request bodies of at most five bytes. Larger bodies are rejected
before the `POST /upload` handler runs.

The first constructor argument configures TCP: `max_connections = 65535`,
`inactivity_period = 60s`, and `max_pending_sources = 32`. The second configures
HTTP: `max_pending_requests = 128` and `max_pending_response_bytes = 1 MiB`.
These are the defaults; the example spells them out alongside its five-byte
request-body limit. The transport send-byte budget keeps its default **1 MiB**.

Admission applies across one server's workers; the reader and HTTP queue limits
apply per connection. The bundled TCP and TLS transports pause request input
under pressure and resume after draining; a hard overflow closes the connection.
Zero disables each of the five limits above. The transport send-byte budget
must stay positive. See [limits and operation](../../../../docs/guide/06-limits-and-operation.md#bound-work-in-flight)
for accounting, hysteresis and the boundaries these settings cover.

## Try it

```sh
curl -i --data-binary "12345" http://localhost:8080/upload
curl -i --data-binary "123456" http://localhost:8080/upload
```

The first request returns `200 OK` with `accepted`. The second returns
`413 Content Too Large`.
