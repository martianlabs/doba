# Examples

The examples are grouped by public API area. Configure and build the project
from the repository root before running an executable:

```text
cmake -S . -B out/build/release -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/release
```

- [Common utilities](common/README.md) demonstrates reusable `common` APIs.
- [Custom protocol](protocol/custom_protocol/README.md) uses doba's TCP
  transport.
- [Custom transport](transport/server/custom_transport/README.md) runs doba's
  HTTP/1.1 engine in memory.
- [HTTP/1.1](http/v11/README.md) demonstrates the HTTP server API.

