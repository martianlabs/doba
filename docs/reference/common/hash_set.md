<a id="hash-set-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/hash_set/index-dark.svg">
    <img src="../../../resources/docs/reference/common/hash_set/index.svg" alt="Index for hash_set.h">
  </picture>
</h1>

**Header:** [include/common/hash_set.h](../../../include/common/hash_set.h)

This header declares the types and operations listed below.

**common**

- [hash_set](#alias-hash-set) · *alias*

<a id="alias-hash-set"></a>

## hash_set

🟠 **ALIAS** · *common*

```cpp
template <typename KEty>
using hash_set = std::unordered_set<KEty, base_hash, base_equal>;
```

Provides a generic hash based set.

**Template parameters:**

- **KEty:** `typename KEty`.

**Source:** [include/common/hash_set.h:41](../../../include/common/hash_set.h#L41)
