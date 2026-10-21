<a href="../../../resources/docs/reference/common/cache/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/cache/title.svg" alt="cache">
</a>
<a href="../../../resources/docs/reference/common/cache/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/cache/title-dark.svg" alt="cache">
</a>

**Header:** [include/common/cache.h](../../../include/common/cache.h)

A concurrent LRU cache independent of protocols and storage. Try the
[generic cache example](../../../examples/common/cache/README.md).

Values are immutable through **all** aliases supplied by loaders. Returned handles
may outlive expiration, eviction, invalidation and the cache itself. Stop/join
callers before destroying the cache. Loaders must not recursively load this cache;
value destructors, hash/equality and clocks must not reenter it. Destructors can run
under the cache mutex, so keep them short. Keys must be copyable and hashable. Hash/equality and clock operations must not throw.

Declared cost is in caller-defined units, not process RSS. Include an upper bound
for the retained value. Map/list/key, allocator and control-block overhead is outside
that budget; external aliases created by loaders remain the caller's responsibility.
Reservation precedes loading. Evicted values pinned by consumers remain charged.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache

[cache_policies](#policies) · [cache_statistics](#stats) · [cache](#cache) · [value_type](#value-type) · [find](#find) · [get_or_load](#load) · [erase](#erase) · [clear](#clear) · [statistics](#statistics)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="policies"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache_policies

```cpp
struct cache_policies { ... };
```

Configuration copied at construction; negative TTL throws **std::invalid_argument**.

| Field | Type / default | Meaning |
| --- | --- | --- |
| max_entries | std::size_t / 1024 | Resident entry limit; zero disables admission. |
| max_cost | std::size_t / 67108864 | Live declared cost limit, including reservations and pinned evicted values; zero disables admission. |
| max_loads | std::size_t / 16 | Concurrent loaders, including invalidated loaders still running; zero disables admission. |
| ttl | std::chrono::milliseconds / 1000 | Starts after successful loading; zero disables automatic expiration. Removal is lazy. |

<a id="stats"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache_statistics

```cpp
struct cache_statistics { ... };
```

All fields are **std::size_t**, initially zero.

| Field | Meaning |
| --- | --- |
| entries | Resident entries, including unvisited expired entries. |
| resident_cost | Cost held by residents. |
| live_cost | Cost of reservations and values retained through managed handles. |
| loads | Loader calls still running, including invalidated calls. |
| waiters | Callers waiting for a same-key load. |

These are resource counters, not hit-rate or process-memory measurements.

<a id="cache"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache

```cpp
template <typename Key, typename Value, typename Clock = std::chrono::steady_clock,
          typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
class cache;
```

**Key** identifies immutable **Value** payloads. **Hash**/**Equal** provide stable key lookup. **Clock** supplies a monotonic now(), time_point and duration compatible with chrono. Construction accepts cache_policies. The cache is noncopyable. Structural operations use a mutex; loaders execute outside it.

<a id="value-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> value_type

```cpp
using value_type = std::shared_ptr<const Value>;
```

An owning immutable handle that retains its declared cost until the last cache-managed owner releases it.

<a id="find"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> find

```cpp
value_type find(const Key& key);
```

**Parameters:** key identifies the value.

**Returns:** A handle on hit, null on miss/expiration. Hits update LRU recency. Expiration does not affect existing consumers.

<a id="load"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_or_load

```cpp
template <typename Loader>
value_type get_or_load(const Key& key, std::size_t cost, Loader&& loader);
```

**Parameters:** key identifies the value; cost upper-bounds its resources; loader is invoked without arguments and returns a pointer convertible to shared_ptr<const Value>. Null means no value.

**Returns:** A hit or loaded value; null if the loader returns null, admission fails,
or the flight is invalidated. Rejected admission never calls the loader. Hits ignore
the supplied cost/loader. Same-key callers share the first loader and declared cost.
Different keys can load concurrently within max_loads.

**Errors:** Loader exceptions reach its caller and all waiting callers. Null values
and failures are not cached. Same-thread same-key recursion throws std::logic_error;
cross-key recursive dependencies can deadlock and are unsupported. Allocation,
key-copy and hash/equality failures may propagate.

Admission evicts LRU residents if needed. Pins can prevent admission even when the
cache has no residents. Invalidating a flight immediately releases its waiters
with null and prevents publication or return of its obsolete value. Loader code
itself is not cancelled. A later call can start a new generation.

<a id="erase"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> erase

```cpp
void erase(const Key& key);
```

**Parameters:** key identifies the resident entry or load.

**Returns:** void. Removes the entry and invalidates a running load. Existing handles remain valid.

<a id="clear"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> clear

```cpp
void clear();
```

**Returns:** void. Removes all residents and invalidates all flights. Active loaders still count toward max_loads, and retained values remain charged.

<a id="statistics"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> statistics

```cpp
[[nodiscard]] cache_statistics statistics() const;
```

**Returns:** A synchronized resource snapshot. No expiration sweep is performed. live_cost can decrease independently as consumers release handles.
