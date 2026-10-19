<a id="hash-base"></a>
<a href="../../../resources/docs/reference/common/hash_base/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/hash_base/index.svg" alt="hash_base">
</a>
<a href="../../../resources/docs/reference/common/hash_base/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/hash_base/index-dark.svg" alt="hash_base">
</a>

**Header:** [include/common/hash_base.h](../../../include/common/hash_base.h)

This struct is needed for the hash_set/hash_map helpers. It performs an ASCII case-insensitive hash.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[ascii_to_lower](#function-ascii-to-lower) ·
[base_hash](#type-base-hash) ·
[base_equal](#type-base-equal)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::base_hash

[is_transparent](#alias-is-transparent)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::base_equal

[is_transparent](#alias-is-transparent-2)

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="function-ascii-to-lower"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ascii_to_lower

**FUNCTION** · *common*

```cpp
static constexpr unsigned char ascii_to_lower(unsigned char value) noexcept;
```

This function converts an ASCII uppercase character to lowercase.

**Parameters:**

- **value:** Value to set or process. Declaration: `unsigned char value`.

**Returns:** A value of type `unsigned char`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/hash_base.h:39](../../../include/common/hash_base.h#L39)

<a id="type-base-hash"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> base_hash

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct base_hash { ... };
```

This struct is needed for the hash_set/hash_map helpers. It performs an ASCII case-insensitive hash.

**Source:** [include/common/hash_base.h:54](../../../include/common/hash_base.h#L54)

<a id="alias-is-transparent"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_transparent

**ALIAS** · *common::base_hash*

```cpp
using is_transparent = void;
```

Alias for `void`.

**Source:** [include/common/hash_base.h:58](../../../include/common/hash_base.h#L58)

<a id="type-base-equal"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> base_equal

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct base_equal { ... };
```

This struct is needed for the hash_set/hash_map helpers. It performs an ASCII case-insensitive comparison.

**Source:** [include/common/hash_base.h:90](../../../include/common/hash_base.h#L90)

<a id="alias-is-transparent-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_transparent

**ALIAS** · *common::base_equal*

```cpp
using is_transparent = void;
```

Alias for `void`.

**Source:** [include/common/hash_base.h:94](../../../include/common/hash_base.h#L94)
