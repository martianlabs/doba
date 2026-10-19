<a id="hash-map-h"></a>
<h1>
  <a href="../../../resources/docs/reference/common/hash_map/index.svg#gh-light-mode-only">
    <img src="../../../resources/docs/reference/common/hash_map/index.svg" alt="Index for hash_map.h">
  </a>
  <a href="../../../resources/docs/reference/common/hash_map/index-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/docs/reference/common/hash_map/index-dark.svg" alt="Index for hash_map.h">
  </a>
</h1>

**Header:** [include/common/hash_map.h](../../../include/common/hash_map.h)

This header declares the types and operations listed below.

**common**

- [hash_map](#alias-hash-map) · *alias*

<a id="alias-hash-map"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> hash_map

**ALIAS** · *common*

```cpp
template <typename KEty, typename VAty>
using hash_map = std::unordered_map<KEty, VAty, base_hash, base_equal>;
```

Provides a generic hash based map.

**Template parameters:**

- **KEty:** `typename KEty`.
- **VAty:** `typename VAty`.

**Source:** [include/common/hash_map.h:41](../../../include/common/hash_map.h#L41)
