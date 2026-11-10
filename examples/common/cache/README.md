# Cache storage

Build and run **common_cache** using the [examples instructions](../../README.md).

```console
cmake --build out/build/msvc-debug --target common_cache
out/build/msvc-debug/examples/common/cache/common_cache.exe
```

The [source](main.cpp) demonstrates the storage used by doba's static-file server
with a small string payload. Two lookups invoke one loader. Invalidation removes
the resident entry and its cost while existing consumers keep **Mars** alive.
A replacement can be admitted before those consumers release their values.

try_get_or_load() invokes an admitted loader synchronously outside the lock.
Requests encountering an active same-key load or the concurrent load limit
bypass loading; the static-file server streams directly in those cases.
Successful loads evict LRU entries to fit the resident entry and cost limits.
Null or failed loaders leave resident entries intact. Invalidation prevents
obsolete loads from publishing over newer ones.

Returned values use the loader's shared_ptr directly. The resident budget excludes
values retained after eviction and loads in progress; it is not a total-memory
bound. Stop/join callers before destroying the cache. Values may outlive it.

See [cache](../../../docs/reference/common/cache.md),
[cache_policies](../../../docs/reference/common/cache_policies.md).
