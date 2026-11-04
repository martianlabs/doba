# Request limits

This example passes HTTP/1.1 `policies` as the second argument to `server`.
It accepts request bodies of at most five bytes. Larger bodies are rejected
before the `POST /upload` handler runs.

The first constructor argument configures TCP: `max_connections = 65535`.
The second configures
HTTP: positive `max_pending_requests = 128`,
`max_pending_response_bytes = 1 MiB`, `max_deferred_input_size = 1 MiB` and
`max_task_steps_per_turn = 64`. These bound engine work and retained state;
both native TCP backends and TLS deliver the engine processing turns.
These are the defaults; the example spells them out alongside its five-byte
request-body limit. The transport send-byte budget keeps its default **1 MiB**.

Admission applies across one server's workers; HTTP queue limits
apply per connection. TCP enforces hard send limits; an overflow closes the
connection. TLS has separate documented plaintext and ciphertext budgets.
Zero disables only the transport connection limit. HTTP pending requests,
retained bytes and task-step budgets must remain positive. The single terminal
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
This example registers synchronous handlers. Cooperative routes are supported
by the same engine without a switch; see [async_routes](../async_routes/README.md).
For TLS use `https_server<>` and account for its finite source budget.

Neither TCP backend provides inactivity expiry or a pressure/resume handshake.
Use max_buffered_send_bytes and max_connections as hard transport limits.
