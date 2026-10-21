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
