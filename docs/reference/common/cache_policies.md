<a href="../../../resources/docs/reference/common/cache_policies/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/cache_policies/title.svg" alt="cache_policies">
</a>
<a href="../../../resources/docs/reference/common/cache_policies/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/cache_policies/title-dark.svg" alt="cache_policies">
</a>

**Header:** [include/common/cache_policies.h](../../../include/common/cache_policies.h)

Limits copied when constructing the [cache](cache.md). Zero in any limit
disables admission. Existing consumers retain their shared values.

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
struct cache_policies {
  std::size_t max_entries = 1024;
  std::size_t max_cost = 512 * 1024 * 1024;
  std::size_t max_loads = 16;
};
```

<a id="max-entries"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_entries

Maximum resident entries.

<a id="max-cost"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_cost

Maximum resident declared cost: 512 MiB by default when costs are bytes.
Enforced at publication by LRU eviction. Excludes running loads and values
retained by responses after eviction, as well as key, allocator and
control-block overhead. It does not bound process RSS.

<a id="max-loads"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_loads

Maximum simultaneous cache loaders, including invalidated loaders still running.
A same-key active load or exhausted load limit causes immediate bypass of loading.
There is no expiration policy; the static-file server revalidates metadata.
