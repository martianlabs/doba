<a id="hash-map"></a>
<a href="../../../resources/docs/reference/common/hash_map/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/hash_map/title.svg" alt="hash_map">
</a>
<a href="../../../resources/docs/reference/common/hash_map/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/hash_map/title-dark.svg" alt="hash_map">
</a>

**Header:** [include/common/hash_map.h](../../../include/common/hash_map.h)

This header declares the types and operations listed below.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[hash_map](#alias-hash-map)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="alias-hash-map"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> hash_map

**ALIAS** · *common*

```cpp
template <typename KEty, typename VAty>
using hash_map = std::unordered_map<KEty, VAty, detail::base_hash, detail::base_equal>;
```

Provides a generic hash based map.

**Template parameters:**

- **KEty:** `typename KEty`.
- **VAty:** `typename VAty`.

**Source:** [include/common/hash_map.h:41](../../../include/common/hash_map.h#L41)
