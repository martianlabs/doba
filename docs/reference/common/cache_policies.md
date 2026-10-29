<a href="../../../resources/docs/reference/common/cache_policies/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/cache_policies/title.svg" alt="cache_policies">
</a>
<a href="../../../resources/docs/reference/common/cache_policies/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/cache_policies/title-dark.svg" alt="cache_policies">
</a>

**Header:** [include/common/cache_policies.h](../../../include/common/cache_policies.h)

Admission limits and expiration settings for the [generic cache](cache.md). Costs use caller-defined units rather than process RSS. See the [generic example](../../../examples/common/cache/README.md).

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache_policies

[cache_policies](#type-cache-policies) · [max_entries](#max-entries) · [max_cost](#max-cost) · [max_loads](#max-loads) · [ttl](#ttl)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-cache-policies"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> cache_policies

```cpp
struct cache_policies { ... };
```

Configuration copied when constructing a cache. Changing the original configuration afterwards does not affect that cache. Negative TTL is rejected by the cache constructor with std::invalid_argument.

<a id="max-entries"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_entries

```cpp
std::size_t max_entries = 1024;
```

Maximum resident entries. Zero disables admission; existing consumers are unaffected by eviction.

<a id="max-cost"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_cost

```cpp
std::size_t max_cost = 512 * 1024 * 1024;
```

Maximum live declared cost, including load reservations and values retained by consumers after eviction. Zero disables admission. The default is 536870912 caller-defined cost units (512 MiB when costs are expressed in bytes); map/list/key, allocator and control-block overhead is excluded.

<a id="max-loads"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_loads

```cpp
std::size_t max_loads = 16;
```

Maximum simultaneous loader calls, including invalidated loaders still finishing. Zero disables admission. Same-key waiters share a loader instead of consuming another slot.

<a id="ttl"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ttl

```cpp
std::chrono::milliseconds ttl{1000};
```

Entry lifetime beginning after a successful load, in milliseconds. Zero disables automatic expiration. Expired entries are removed lazily; existing consumers keep their values. Negative durations are invalid.
