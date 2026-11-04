# Ordered response record

Header: [pending_response.h](../../../../../include/protocol/http/v11/pending_response.h).

`detail::pending_response<RQty, RSty>` is one stable request position owned
by the engine. Its properties are:

- `handler`: optional async request/frame owner; absent for queued sync output.
- `interim`: owned ordered 100 Continue head, at most once per request.
- `head`, `body`: owned final serialized prefix/inline bytes awaiting acceptance.
- `source`: the same optional reader accepted from a synchronous response.
- `complete`: final output is available for its ordered emission frontier.
- `close`: this final response is an ordered connection-close boundary.
- `source_sent`: keep the record alive while its accepted terminal source drains.

The engine checks aggregate byte limits before copying serialized views. It
charges interim/head/body logical bytes until transfer/discard. Reader backing
storage, string capacity and arbitrary handler allocations are separate from
this byte budget. Retained request/response objects and task frames follow
their own policies; their storage is not charged again as serialized copies.
The in-flight position limit still counts completed records
waiting for emission and the source currently draining. There is no mutex or
cross-thread completion publication on these records. See [engine](engine.md).
