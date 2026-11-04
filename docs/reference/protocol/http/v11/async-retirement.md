# Async implementation retirement inventory

This inventory separates obsolete mechanisms from the ownership required by the
current single engine with concurrent cooperative handlers. It describes source
changes through block 8 of [the plan](../../../../../ASYNC.md); local command logs,
source snapshots and qualified results are in out/async-processing/block-8.
It is not a statement that the later release/performance gates have completed.

## Retired mechanisms and coverage

| Mechanism | Consumer audit and action | Current coverage |
| --- | --- | --- |
| Transport execution work/capacity/receiver contracts | No production caller remained; only a fake Windows test engine implemented the removed setter. Deleted declarations and their documentation. | The [portable factory test](../../../../../tests/integration/transport/server/tcp_factory_tests.cpp) requires only the current engine signatures and verifies factory state, echo, disconnect and one/two workers on each OS. |
| Pressure watermark helper | Its sole caller was its own watermark test; no engine or transport used it. Removed helper and that obsolete test. | Hard TX caps, partial batches, reader ownership and HTTP byte/admission limits retain their existing assertions and mutation detectors. |
| Mixed operating-state header | Its two live classes still implement Windows admission. Replaced the header with [connection_budget](../../../../../include/transport/server/connection_budget.h) and [connection_reservation](../../../../../include/transport/server/connection_reservation.h), one class each. Removed its unused documentation title assets. | Admission move/unwind/concurrency tests, native Windows admission/lifetime probes and the private-copy admission mutation remain active. Atomic operations, thresholds and move/release behavior are unchanged. |
| Legacy execution/flow probe | Tested fake execution and flow setters that the current engine does not expose. Removed the Windows-only fixture instead of keeping public contracts just for the fixture. | Portable factory ownership/state cases cover the useful behavior on both backends. Current native turn tests cover actual processing. |
| Activity/pressure microbenchmarks | The activity program referenced an absent connection-progress type. The other program only modeled retired hooks and silently skipped them for the current engine. Deleted both programs, their CMake target/options and stale README. | Live comparisons remain in the existing HttpArena adapter; final async correctness and throughput work is scheduled in blocks 9/10. No replacement synthetic performance claim is made. |
| Old flow-wakeup test name | Cases already test processing_result turns rather than pressure subscriptions. Renamed the file to [processing_turn_tests.cpp](../../../../../tests/unit/protocol/http/v11/processing_turn_tests.cpp); bodies retained byte for byte. | The same turn, owned-frame, cancellation and response-order cases remain in the ordinary unit binary. |
| Old engine_sync/engine_async and single-active gate | Already retired in block 4; no current production or build consumer exists. No compatibility wrappers or one-active-per-connection gate were added back. | Concurrent same-connection engine tests, native TCP/TLS tests and ordered mixed/source output remain active. |

The admission mutation now targets connection_reservation.h. The detector name
and exact capacity assertion stay unchanged; the campaign must compile the
mutant successfully, then fail that assertion. A missing anchor, build failure,
unrelated assertion or sanitizer report cannot count as detection.

Block 9's global inventory also retired the optional protocol completion
contract: no engine implemented it, and only two negative static assertions
referenced it. Both required engine entry points and factory checks are unchanged.
The similarly named Windows socket-completion probe is a current test fixture.

## Current ownership retained deliberately

- [task_scheduler](../../../../../include/common/task_scheduler.h) is consumed by
  public standalone task execution and the local controlled task owner. It is
  not a transport executor. Removing it would break valid common::task usage.
- [task_execution](../../../../../include/common/task_execution.h) owns each
  managed frame and supports local ready/deadline state, errors and cancellation.
  It never publishes handles to native packets. It remains unchanged here.
- [pending_response](pending_response.md), lazy ordered engine records, retained
  raw input, byte accounting and source-drain state implement finite admission,
  stable request/frame lifetime and HTTP response order. Later handlers advance
  while earlier ones wait; completed successors still wait to emit. This state
  is required and remains unchanged.
- Linux ready/deadline registrations and Windows generic IOCP packets/group
  timers implement processing_result delivery, not application jobs. Eager
  cancellation, exact connection owners/versions and serialized engine entries
  remain necessary for late notifications and native completion.
- Windows admission reservations, live contexts, OVERLAPPED owners and lifecycle
  synchronization are current native-I/O safety mechanisms. Only the admission
  header organization changes; socket send/receive, close and stop algorithms
  remain unchanged.
- TLS forwards snapshots and serializes SSL through those same transport entries.
  It owns no separate executor or callback thread and is unchanged in block 8.

No protocol stop hook, flow-control setter, completion observer, transport job
submission API, pressure sampler, weak_ptr or try_lock is introduced. Rules for
serialized entries and managed awaiters still apply. The global driver/examples
inventory and final qualification/comparisons remain separate blocks 9 and 10.
