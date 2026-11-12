<a id="hash-set"></a>
<a href="../../../resources/docs/reference/common/hash_set/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/hash_set/title.svg" alt="hash_set">
</a>
<a href="../../../resources/docs/reference/common/hash_set/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/hash_set/title-dark.svg" alt="hash_set">
</a>

**Header:** [include/common/hash_set.h](../../../include/common/hash_set.h)

This header declares the types and operations listed below.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[hash_set](#alias-hash-set)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="alias-hash-set"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> hash_set

**ALIAS** · *common*

```cpp
template <typename KEty>
using hash_set = std::unordered_set<KEty, detail::base_hash, detail::base_equal>;
```

An ASCII-case-insensitive string-key set used by doba. The hash and equality
objects fold `A` through `Z` only; embedded zero bytes and bytes above ASCII
remain significant. This is not locale or Unicode case folding.

`KEty` must work with the `std::string_view` hash/equality operations, such as
`std::string` or `std::string_view`. Transparent lookup accepts string views
without building a key. Equivalent spellings identify the same element.

View keys require stable, unchanged backing bytes while stored. Other
operations and iterator invalidation follow `std::unordered_set`; the alias
adds no locking.

**Template parameters:**

- **KEty:** `typename KEty`.

**Source:** [include/common/hash_set.h](../../../include/common/hash_set.h)
