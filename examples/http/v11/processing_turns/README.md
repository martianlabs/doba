# Concurrent HTTP processing turns

Build and run `http_processing_turns` with examples enabled. The executable
uses one HTTP engine and a serialized in-memory adapter, starts three async
parametrized handlers on one pipeline and verifies ordered output `1 2 3`.
Every handler begins its own short sleep before any response is emitted.
The original RX storage is overwritten after delivery to exercise ownership.

The adapter supplies immediate/deadline/output-drain turns on its calling
thread. Its `sleep_until` is only the standalone example's wait mechanism;
it is not part of the engine or a transport application executor.

No async activation switch or alternative engine type is required. Plain
Linux/Windows TCP supplies native turns; see
[async_routes](../async_routes/README.md).
TLS forwards these same native turns; see the
[HTTPS example](../https_hello_world/README.md).
This example stays portable by supplying engine turns itself.

See [engine](../../../../docs/reference/protocol/http/v11/engine.md).
