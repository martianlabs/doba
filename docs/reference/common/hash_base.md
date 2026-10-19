<a id="hash-base-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/hash_base/index-dark.svg">
    <img src="../../../resources/docs/reference/common/hash_base/index.svg" alt="Index for hash_base.h">
  </picture>
</h1>

**Header:** [include/common/hash_base.h](../../../include/common/hash_base.h)

This struct is needed for the hash_set/hash_map helpers. It performs an ASCII case-insensitive hash.

**common**

- [ascii_to_lower](#function-ascii-to-lower) · *function*
- [base_hash](#type-base-hash) · *type*

**common::base_hash**

- [is_transparent](#alias-is-transparent) · *alias*
- [operator()](#operator-operator) · *operator*

**common**

- [base_equal](#type-base-equal) · *type*

**common::base_equal**

- [is_transparent](#alias-is-transparent-2) · *alias*
- [operator()](#operator-operator-2) · *operator*

<a id="function-ascii-to-lower"></a>

## ascii_to_lower

🟠 **FUNCTION** · *common*

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

## base_hash

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
struct base_hash { ... };
```

This struct is needed for the hash_set/hash_map helpers. It performs an ASCII case-insensitive hash.

**Source:** [include/common/hash_base.h:54](../../../include/common/hash_base.h#L54)

<a id="alias-is-transparent"></a>

## is_transparent

🟠 **ALIAS** · *common::base_hash*

```cpp
using is_transparent = void;
```

Alias for `void`.

**Source:** [include/common/hash_base.h:58](../../../include/common/hash_base.h#L58)

<a id="operator-operator"></a>

## operator()

🟠 **OPERATOR** · *common::base_hash*

```cpp
std::size_t operator()(std::string_view value) const noexcept;
```

Implements operator() for base_hash.

**Returns:** The operator() count or size.

**Errors:** Declared `noexcept`.

**Source:** [include/common/hash_base.h:62](../../../include/common/hash_base.h#L62)

<a id="type-base-equal"></a>

## base_equal

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
struct base_equal { ... };
```

This struct is needed for the hash_set/hash_map helpers. It performs an ASCII case-insensitive comparison.

**Source:** [include/common/hash_base.h:90](../../../include/common/hash_base.h#L90)

<a id="alias-is-transparent-2"></a>

## is_transparent

🟠 **ALIAS** · *common::base_equal*

```cpp
using is_transparent = void;
```

Alias for `void`.

**Source:** [include/common/hash_base.h:94](../../../include/common/hash_base.h#L94)

<a id="operator-operator-2"></a>

## operator()

🟠 **OPERATOR** · *common::base_equal*

```cpp
bool operator()(std::string_view lhs, std::string_view rhs) const noexcept;
```

Implements operator() for base_equal.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/hash_base.h:98](../../../include/common/hash_base.h#L98)
