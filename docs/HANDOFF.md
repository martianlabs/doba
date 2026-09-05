# doba documentation

Doba is a C++20 header-only server framework with an IOCP backend on Windows
and an epoll backend on Linux.

## Reading guide

| Document | Content and source of truth |
| --- | --- |
| [Architecture](ARCHITECTURE.md) | Architectural principles, protocol separation, and memory and execution design. |
| [Development](DEVELOPMENT.md) | Requirements, build, style, and change validation. |
| [Quality](QUALITY.md) | Engineering rules, verification gates, and release requirements. |
| [Backlog](BACKLOG.md) | All outstanding work, dependencies, and release criteria. |

The implementation has unit tests, integration tests over real sockets, and
cross-platform CI with sanitizers. The next beta's scope and exit criteria
are maintained in the [release backlog](BACKLOG.md#release-target).

The [project README](../README.md) introduces the library and how to consume
it. The [examples](../examples/README.md) document specific API use cases.

## Maintenance

Update each fact in its reference document. Keep architecture focused on
design principles and quality focused on engineering rules. Record verification
results with the corresponding change or release. When closing a backlog
item, update any affected contract, the inventory, and its links.
Examples and tool-specific instructions remain alongside their components
and are linked from this documentation.
