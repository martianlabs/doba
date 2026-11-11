# Retired execution APIs

doba has one HTTP engine for synchronous and cooperative coroutine handlers.
The old split engines, transport job submission and pressure/wakeup setters
are no longer part of the API. There are no compatibility wrappers for them.

## Current boundary

- An argument-free factory creates one engine per connection. See the
  [engine contract](../../contracts.md) and
  [factory test](../../../../../tests/integration/transport/tcp_factory_tests.cpp).
- Input and processing calls return a complete
  [processing_result](../../processing_result.md). Send and close callbacks
  run only inside those serialized calls.
- The engine owns task frames, request lifetimes and response order.
  [task_execution](../../../common/task_execution.md) supports *yield()* and
  *sleep_for()*; transports do not receive coroutine handles or application jobs.
- Linux ready/deadline registrations and Windows IOCP packets deliver engine
  turns. Cancellation removes registrations before the engine is destroyed.
  See [Linux processing](../../../transport/tcp_linux_processing.md) and
  [Windows processing](../../../transport/tcp_windows_processing.md).
- TLS forwards processing requests through the same TCP entries. It has no
  separate application executor.

## Admission and validation

Windows admission uses separate
[connection_budget](../../../../../include/transport/connection_budget.h) and
[connection_reservation](../../../../../include/transport/connection_reservation.h)
headers. Reservations keep their slot until pending I/O and the last context
owner release it.

The [CI inventory](../../../../../tests/ci-inventory.md) covers concurrent
handlers, ordered output, native cancellation and publication failures, TLS
processing and executable examples. The private-copy admission mutation targets
*connection_reservation.h*.

Activity/pressure microbenchmarks and fixtures for the removed setters were
deleted. Current benchmark adapters live under
[benchmarks/](../../../../../benchmarks/README.md); published measurements retain
their own dates and qualification limits.
