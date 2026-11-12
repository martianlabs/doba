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

An ASCII-case-insensitive string-key table used by doba. The hash and equality
objects fold `A` through `Z` only; embedded zero bytes and bytes above ASCII
remain significant. This is not a locale or Unicode case-folding container.

`KEty` must work with the `std::string_view` hash/equality operations, such as
`std::string` or `std::string_view`; arbitrary key types are not supported by
this alias. Transparent lookup accepts string views without building a key.
Equivalent spellings refer to the same entry.

Owning keys keep their bytes. View keys require stable, unchanged backing
storage for the entire time they remain in the map. Other operations and
iterator invalidation follow `std::unordered_map`; the alias adds no locking.

**Template parameters:**

- **KEty:** `typename KEty`.
- **VAty:** `typename VAty`.

**Source:** [include/common/hash_map.h](../../../include/common/hash_map.h)
