<a href="../../../resources/docs/reference/common/cache/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/cache/title.svg" alt="cache">
</a>
<a href="../../../resources/docs/reference/common/cache/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/cache/title-dark.svg" alt="cache">
</a>

**Header:** [include/common/cache.h](../../../include/common/cache.h)

Concurrent resident-bounded LRU storage used by doba's static-file server.
Keys are strings; values are immutable through every alias supplied by a loader.
See the [example](../../../examples/common/cache/README.md).

Resident bookkeeping is declared separately in [cache_entry.h](cache_entry.md).

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="cache"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache

```cpp
template <typename VAty>
class cache;
```

Construction copies [cache_policies](cache_policies.md). The cache is noncopyable.
Callers must finish before cache destruction; returned values may outlive it.
Resident entries, loaders in progress, and response-owned values have separate
lifetimes. Only resident entries count toward max_cost.

<a id="value-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> value_type

```cpp
using value_type = std::shared_ptr<const VAty>;
```

The loader's shared ownership is returned directly. Eviction and invalidation
release the cache's reference without affecting existing responses.

<a id="find"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> find

```cpp
value_type find(const std::string& key);
```

Returns the resident value or null. Lookups share a read lock. Sequential hits
promote the entry in LRU order; overlapping hits may coalesce promotions using
a nonblocking exclusive lock. The cache has no TTL: the static-file server
checks filesystem metadata on each request.

<a id="try-load"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_get_or_load

```cpp
template <typename LDty>
value_type try_get_or_load(const std::string& key, std::size_t cost, LDty&& loader);
```

A hit returns its value without invoking the loader. A miss returns null without
loading if admission is disabled, cost exceeds max_cost, max_loads is reached,
or the key already has an active load. It never waits for another loader,
though cache operations still acquire locks.

An admitted loader runs synchronously outside the cache lock and returns a
shared_ptr<const VAty>; null means there is no value to cache. Exceptions
propagate only to that caller, after releasing its load slot. A null result or
an exception thrown by the loader does not evict residents. Same-key recursion
also bypasses loading. Allocating the cache entry can throw after LRU eviction;
publication does not provide rollback of those evictions.

A successful current load evicts least-recently-used entries as necessary and
publishes within both resident limits. Cost uses the server's declared byte
estimate, excluding allocator, key and control-block overhead. Loads in progress
and evicted values retained by responses do not count toward max_cost; this is
not a total-memory bound.

The internal load identifier never wraps. Exhausting it disables further miss
admission for that cache object; `erase()` and `clear()` do not reset it.

<a id="erase"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> erase(key)

```cpp
void erase(const std::string& key);
```

Removes the resident entry and cancels publication by its current load. That
loader continues to count toward max_loads until it finishes; a successful
obsolete load returns null and cannot disturb a newer load or entry.

<a id="erase-expected"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> erase(key, expected)

```cpp
bool erase(const std::string& key, const value_type& expected);
```

Removes the entry only if it still contains the observed value. Returns whether
it was removed. Newer entries and loads remain untouched.

<a id="clear"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> clear

Removes every resident entry and cancels publication by all current loads.
Existing responses retain their values; running loaders keep their slots.
