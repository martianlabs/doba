# cache_entry

**Header:** [include/common/cache_entry.h](../../../include/common/cache_entry.h)

Bookkeeping used by [cache](cache.md). Applications use the cache operations
and returned values rather than constructing entries themselves.

```cpp
template <typename VAty>
struct cache_entry {
  std::shared_ptr<const VAty> value;
  std::size_t cost;
  std::list<std::string>::iterator recency;
};
```

`value` keeps the cached data alive. `cost` is the caller's declared cost, and
`recency` points into the cache's LRU list. Removing an entry releases the
cache's reference; values already held by responses remain valid.
