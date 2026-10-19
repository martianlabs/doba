<a id="hash-map-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/hash_map/index-dark.svg">
    <img src="../../../resources/docs/reference/common/hash_map/index.svg" alt="Index for hash_map.h">
  </picture>
</h1>

**Header:** [include/common/hash_map.h](../../../include/common/hash_map.h)

This header declares the types and operations listed below.

**common**

- [hash_map](#alias-hash-map) · *alias*

<a id="alias-hash-map"></a>

## hash_map

🟠 **ALIAS** · *common*

```cpp
template <typename KEty, typename VAty>
using hash_map = std::unordered_map<KEty, VAty, base_hash, base_equal>;
```

Provides a generic hash based map.

**Template parameters:**

- **KEty:** `typename KEty`.
- **VAty:** `typename VAty`.

**Source:** [include/common/hash_map.h:41](../../../include/common/hash_map.h#L41)
