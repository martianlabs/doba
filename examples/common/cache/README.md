# Generic cache

Build and run **common_cache** using the [examples instructions](../../README.md).
The [source](main.cpp) caches a planet name without depending on HTTP or files.

```console
cmake --build out/build/msvc-debug --target common_cache
out/build/msvc-debug/examples/common/cache/common_cache.exe
```

Two lookups invoke one loader. Invalidating the key removes the entry, while
existing consumers keep **Mars** alive. Releasing them returns the charged cost
to zero. See the [API reference](../../../docs/reference/common/cache.md).

The cost argument is a caller-defined upper bound in consistent units. Capacity
rejection returns a null pointer without running the loader. Loaders run outside
the cache lock; concurrent callers for the same key share their result or error.
Do not recursively load the cache from a loader. Stop/join callers before
destroying it; returned values may outlive it.

Admission settings are declared in [cache_policies.h](../../../include/common/cache_policies.h);
resource counters are declared in [cache_statistics.h](../../../include/common/cache_statistics.h).
Both headers are self-contained and are also included by cache.h.

Call try_get_or_load() when a protocol worker should fall back rather than wait
for another caller's active load. Admitted loads remain synchronous. The
conditional erase(key, expected) overload removes an observed value only while
it is still resident, preserving newer generations published by other callers.

Immutable lookups share a read lock. LRU promotions use a nonblocking exclusive
lock, so concurrent hits can coalesce recency updates instead of waiting for one
another. Sequential accesses retain LRU order. Custom clocks, hashers and equality
functions must support concurrent calls.
