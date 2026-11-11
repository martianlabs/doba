# <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Connection admission

Internal headers:
[connection_budget.h](../../../include/transport/connection_budget.h)
and [connection_reservation.h](../../../include/transport/connection_reservation.h).
These declarations are implementation details, not supported public API.

Windows acquires one shared admission credit before constructing the engine.
The context retains its reservation while native RX/TX or processing packets
still own it, including operations completing after socket closure. A moved
reservation transfers its single release obligation. Its final owner releases
exactly once; failed setup unwinds the same ownership chain.

connection_budget contains the immutable maximum (zero means unlimited) and
an atomic count of owned reservations. connection_reservation retains that
budget, rejects exhausted/overflowed counters and acquires with compare-exchange.
It preserves the existing atomic ordering and move/release behavior.
Linux partitions admission by worker and does not use this shared counter.

No pressure watermark, activity scan or application execution capability is
part of these headers. The transport's live set, native operation owners and
processing registration owners remain necessary for real connection lifetime.

[Admission unit cases](../../../tests/unit/transport/operating_policies_tests.cpp)
cover move, unwind, concurrent capacity and release. The
[policy mutation campaign](../../../tests/mutation/run_policy_detection.cmake)
removes the guard in its private copy and requires the capacity assertion to
fail. Native [Windows admission cases](../../../tests/integration/transport/admission_windows_tests.cpp)
exercise the actual shared transport limit.
