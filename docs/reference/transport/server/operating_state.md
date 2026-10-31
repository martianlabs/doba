<a id="operating-state"></a>
<a href="../../../../resources/docs/reference/transport/server/operating_state/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/operating_state/title.svg" alt="operating_state">
</a>
<a href="../../../../resources/docs/reference/transport/server/operating_state/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/operating_state/title-dark.svg" alt="operating_state">
</a>

**Header:** [include/transport/server/operating_state.h](../../../../include/transport/server/operating_state.h)

Implementation support for doba. These declarations are internal and are not part of the supported public API.

[Internal implementation](#implementation)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="implementation"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Internal implementation

**INTERNAL HEADER**

Use the public TCP transport and policy types. These declarations are internal.

connection_budget and connection_reservation implement shared Windows admission.
A reservation is acquired before engine construction and released once by its
final owner, including I/O that survives socket closure. Moving a reservation
transfers that release obligation. Linux partitions admission by worker and does
not use this shared budget.

update_pressure is used by independent HTTP engine logic and its tests. Its
presence here does not install pressure queries, input wakeups or an application
executor in the TCP transports. See the
[policy reference](tcp_policies.md) and [runtime contract](tcp.md#runtime-contract).
