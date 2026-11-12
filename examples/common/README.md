<a name="common-utilities"></a>
<h1>
  <a href="../../resources/examples/common/h1-common-utilities.svg#gh-light-mode-only">
    <img src="../../resources/examples/common/h1-common-utilities.svg" alt="Common utilities">
  </a>
  <a href="../../resources/examples/common/h1-common-utilities-dark.svg#gh-dark-mode-only">
    <img src="../../resources/examples/common/h1-common-utilities-dark.svg" alt="Common utilities">
  </a>
</h1>

These examples demonstrate public APIs in `martianlabs::doba::common` that can
be used independently of HTTP. Build them from the repository root as
described in the [examples index](../README.md), then run the target named by
each directory.

| Example | Demonstrates |
| --- | --- |
| `byte_storage` | In-memory storage that spills to a temporary file. |
| `writer` | Incremental writing and ownership transfer. |
| `reader` | Borrowed byte reading and end-of-input handling. |
| [task_execution](task_execution/README.md) | Controlled coroutine ownership. |
| `filesystem` | Confined file opening and direct reading with owned handles. |
| [cache](cache/README.md) | Generic concurrent caching, invalidation and retained ownership. |
| `date_server` | Cached HTTP-date production and lifecycle. |

