# Controlled task execution

Header: [task_execution.h](../../../include/common/task_execution.h).

`common::task_execution` exclusively owns one `common::task<void>` frame.
It has no thread, queue, timer registration, transport reference or mutex.
The caller supplies serialized turns and uses the reported readiness/deadline
to decide when another turn is needed. This is the ownership primitive for
the current async processor. Native TCP workers provide serialized engine
turns on Linux and Windows; TLS forwards them without owning task frames.

## Ownership and limits

Construction consumes an unstarted task without running its body. A moved-from
or already-started task throws `std::invalid_argument`. The owner cannot be
copied or moved: its local scheduler callbacks refer to its stable address.
No coroutine handle escapes through this API.

Only `common::yield()` and `common::sleep_for()` are supported awaiters.
An unsupported await expression throws `std::logic_error` before invoking its
awaiter, including an immediately ready awaiter. An uncaught exception is
retained for the caller. Awaiter types cannot bypass this restriction through
an implicit conversion to a supported awaiter. There is no nested-task await
or foreign I/O resumption in this stage.

The caller must serialize **all** inspection, resumption, cancellation and
destruction with execution of the coroutine body. Reentrant calls and frame
destruction from inside that body are forbidden. Sequential migration between
threads is supported; physical-thread affinity is not required. The coroutine
must not arrange external resumption or access frame-owned state after
cancellation. Coroutine parameters that borrow external objects require those
objects to outlive the owner. Destructors must not throw.

## Operations

| Operation | Meaning |
| --- | --- |
| `task_execution(task<void>)` | Adopt one lazy frame; initially ready. |
| `resume(now = clock::now())` | Run once if ready or its deadline is due. |
| `ready()` | Initial entry or a yield awaits a caller-supplied turn. |
| `deadline()` | Pending positive sleep deadline, otherwise empty. |
| `done()` | Final suspension reached, or the frame was cancelled. |
| `error()` | Exception after completion; empty while pending/cancelled. |
| `cancel()` | Destroy the frame and clear pending state; idempotent. |

`clock` is `std::chrono::steady_clock`. `resume` returns whether it entered the
frame. It does nothing for a cancelled/completed frame or a not-yet-due sleep.
It clears the previous pending state before entry. A yield requests another
turn; it never recursively resumes. A positive sleep uses the real monotonic
clock at suspension and saturates at `clock::time_point::max()` on overflow.
Zero and negative sleeps continue in the same turn.

The `now` argument is an eligibility snapshot, not an injected clock for the
coroutine body. An engine must use a fresh monotonic snapshot before resuming.
Tests can check the exact eligibility boundary using the reported deadline
without sleeping. A new sleep is always computed from the real clock.

Completed frames remain at final suspension until cancellation or destruction.
Their parameters and exception remain owned, so the caller can finish handling
the result before releasing the frame. No completion callback is invoked in
this ownership model. Cancellation destroys local objects and parameters;
it does not resume the body or execute code after the suspended await.

The local scheduler callbacks only update readiness/deadline inside the
current resume call. They neither queue the handle nor run application work.
The caller must remove its own pending turn/timer registrations before
destroying the owner. `ready()` and `deadline()` are mutually exclusive.

Standalone [task::start](task.md) retains its detached lifecycle and custom
awaiters. See the [example](../../../examples/common/task_execution/README.md)
and [tests](../../../tests/unit/common/task_execution_tests.cpp).
