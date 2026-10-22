<a href="../../../resources/docs/reference/common/cache_statistics/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/cache_statistics/title.svg" alt="cache_statistics">
</a>
<a href="../../../resources/docs/reference/common/cache_statistics/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/cache_statistics/title-dark.svg" alt="cache_statistics">
</a>

**Header:** [include/common/cache_statistics.h](../../../include/common/cache_statistics.h)

Resource counters returned by [cache::statistics()](cache.md#statistics). Costs use the units declared by the loader. These counters are not process-memory or hit-rate measurements.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache_statistics

[cache_statistics](#type-cache-statistics) · [entries](#entries) · [resident_cost](#resident-cost) · [live_cost](#live-cost) · [loads](#loads) · [waiters](#waiters)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-cache-statistics"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache_statistics

```cpp
struct cache_statistics { ... };
```

A synchronized cache resource snapshot. All fields are std::size_t and initially zero. Live cost can decrease as consumers release values, independently of subsequent cache operations.

<a id="entries"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> entries

```cpp
std::size_t entries = 0;
```

Number of resident entries, including expired entries not yet visited. Reading statistics does not sweep expired entries.

<a id="resident-cost"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> resident_cost

```cpp
std::size_t resident_cost = 0;
```

Declared cost currently held by resident entries.

<a id="live-cost"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> live_cost

```cpp
std::size_t live_cost = 0;
```

Declared cost of active load reservations and values retained anywhere through managed handles, including evicted entries pinned by consumers.

<a id="loads"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> loads

```cpp
std::size_t loads = 0;
```

Loader calls still running, including invalidated calls awaiting completion.

<a id="waiters"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> waiters

```cpp
std::size_t waiters = 0;
```

Callers currently waiting for an existing load of the same key.
