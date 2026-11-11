# <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Processing turns

Header: [processing_result.h](../../../include/protocol/processing_result.h).
The structural engine interface is defined in
[contracts.h](../../../include/protocol/contracts.h).

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Implementation status

Linux and Windows TCP deliver immediate, monotonic-deadline and output-drain
processing turns on their transport workers. TLS forwards the same complete
snapshots and executes engine turns through those serialized entries.
Coroutine frames remain engine-owned; no transport accepts application jobs
or externally resumed handles.

See the [Linux delivery design](../transport/tcp_linux_processing.md) and
[Windows delivery design](../transport/tcp_windows_processing.md).

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Entry points and ownership

```cpp
processing_result on_bytes_received(std::string_view input);
processing_result on_processing_turn(bool output_idle);
```

Both entry points return processing needs directly. There is no context object,
setter, injected executor or query after the call. The factory still takes no
arguments and creates a separate engine for each connection.

Input is immutable and borrowed only until on_bytes_received returns. The
transport discards the whole block, regardless of processing_result. The result
contains neither a consumed-byte count nor an instruction to enable/pause RX.
The engine must own incomplete input and any data needed after the call. It
must enforce its own retained-input limits; TCP continues its normal operation.

One instance never receives overlapping or recursive entry calls. Different
instances may run concurrently. Windows does not promise a fixed physical
thread. Send/close callbacks remain registered for the engine lifetime and may
be invoked only inside its current serialized input or processing turn.

output_idle is the transport's snapshot before the processing turn: all accepted
inline output and readers have drained, and no send remains outstanding. The
snapshot does not reserve space and can become outdated after the engine emits
new output itself. It does not acknowledge delivery to the peer. Other turns,
including deadlines, are not blocked just because output_idle is false.

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Result properties

| Property | Default | Meaning |
| --- | --- | --- |
| turn_requested | false | Request a later serialized turn, never reentry. |
| turn_at | empty | Request a turn at/after a steady-clock deadline. |
| turn_after_output_drained | false | Request a turn when accepted TX drains. |

The fields are independent. Deadline and output-drain interest can coexist,
for example when an engine waits for output with a timeout. A protocol may
continue accepting ordinary input while also scheduling maintenance turns.
No field denotes an HTTP handler, coroutine, thread or application work item.

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Reconciliation rules

A result is a complete snapshot of current interests, not an append-only list.
The transport reconciles it after either entry point returns. Repeating an
unchanged request does not allocate another registration, postpone a deadline
or enqueue duplicates. Returning an empty result withdraws old interests.

For a pending deadline T, input received before T must not cancel or restart
that wait: the engine returns the same T while it remains necessary. An
immediate request can coexist with a future deadline; it does not erase that
future interest. The engine computes its own earliest logical deadline.

The following rules apply to both native backends. Their processing tests cover
delivery, cancellation, deadlines and publication failures.

| Situation | Required behavior |
| --- | --- |
| Empty result | No autonomous processing requested; ordinary RX remains valid. |
| Immediate turn | Queue a later turn; never invoke recursively. |
| Several ready reasons | Coalesce into a turn without losing pending work. |
| Future deadline | That timed interest cannot activate before T; lateness is possible. |
| Immediate turn before T | Keep the separately requested future interest. |
| Same T after new input | Preserve T; do not create another timer or move T forward. |
| New T or empty deadline | Replace or withdraw the previous timed interest. |
| Output wait and nonempty TX | Keep it pending while ordinary TX makes progress. |
| Output wait and already empty TX | Arrange a later turn; do not wait for a future edge. |
| Deadline while TX is busy | Deliver the turn with output_idle=false. |
| Request during delivery | Preserve the next request; no recursive call or lost wakeup. |
| Close during a turn | Close wins; discard that turn's scheduling result. |
| Old native notification | Validate destination lifetime; never enter a destroyed engine. |

Engines inspect their own state and monotonic time before acting. A turn can
serve several reasons, so it is not permission to blindly resume a frame or
assume a particular deadline/output condition has been met.

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Scope boundaries

Read policy, socket buffers, partial sends and source consumption belong to the
transport. Scheduling a processing turn does not override these operations or
their byte/ownership limits. A terminal reader keeps its existing semantics.

An external completion needs a separate safe notification entry. Neither this
value nor its return path exports an engine pointer or allows arbitrary-thread
protocol execution. External awaiter integration is outside the initial HTTP
scope of managed yield/sleep awaiters.

Stopping invalidates registrations before engine destruction. Pending native
records must outlive native producers and delivery, including IOCP dequeue
before the posting API returns. Timer callbacks never execute application code
or access SSL sessions. No application executor or completion-wait interface
is added to TCP.
