# Common utilities

These examples demonstrate public APIs in `martianlabs::doba::common` that can
be used independently of HTTP. Build them from the repository root as
described in the [examples index](../README.md), then run the target named by
each directory.

| Example | Demonstrates |
| --- | --- |
| `byte_storage` | In-memory storage that spills to a temporary file. |
| `writer` | Incremental writing and ownership transfer. |
| `reader` | Borrowed byte reading and end-of-input handling. |
| `console_logger` | Structured console log levels and formatting options. |
| `date_server` | Cached HTTP-date production and lifecycle. |

