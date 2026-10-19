<a id="writer"></a>
<a href="../../../resources/docs/reference/common/writer/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/writer/index.svg" alt="writer">
</a>
<a href="../../../resources/docs/reference/common/writer/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/writer/index-dark.svg" alt="writer">
</a>

**Header:** [include/common/writer.h](../../../include/common/writer.h)

This class provides a writer interface for writing bytes to a byte_storage backend. It supports writing from a pointer, a string_view, or a span of bytes. It also provides methods to finish the writing process and check the state of the writer.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[writer](#type-writer)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::writer

[write(ptr, len)](#method-write) ·
[write(bytes)](#method-write-2) ·
[write(bytes)](#method-write-3) ·
[finish](#method-finish) ·
[ok](#method-ok) ·
[release](#method-release)

<a id="type-writer"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> writer

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class writer { ... };
```

This class provides a writer interface for writing bytes to a byte_storage backend. It supports writing from a pointer, a string_view, or a span of bytes. It also provides methods to finish the writing process and check the state of the writer.

**Source:** [include/common/writer.h:46](../../../include/common/writer.h#L46)

<a id="method-write"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *common::writer*

```cpp
bool write(const char* ptr, std::size_t len);
```

Writes data.

**Parameters:**

- **ptr:** Pointer to the input or output memory. Declaration: `const char* ptr`.
- **len:** Length in elements or bytes, according to the declared type. Declaration: `std::size_t len`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/writer.h:63](../../../include/common/writer.h#L63)

<a id="method-write-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *common::writer*

```cpp
bool write(std::string_view bytes);
```

Writes data.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::string_view bytes`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/writer.h:71](../../../include/common/writer.h#L71)

<a id="method-write-3"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> write

**METHOD** · *common::writer*

```cpp
bool write(std::span<const std::byte> bytes);
```

Writes data.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::span<const std::byte> bytes`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/writer.h:77](../../../include/common/writer.h#L77)

<a id="method-finish"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> finish

**METHOD** · *common::writer*

```cpp
void finish(std::size_t bytes);
```

Finalizes the buffered data or output stream.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::size_t bytes`.

**Returns:** No value (`void`).

**Source:** [include/common/writer.h:83](../../../include/common/writer.h#L83)

<a id="method-ok"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ok

**METHOD** · *common::writer*

```cpp
[[nodiscard]] bool ok() const noexcept;
```

Reports whether the ok condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/writer.h:91](../../../include/common/writer.h#L91)

<a id="method-release"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> release

**METHOD** · *common::writer*

```cpp
[[nodiscard]] byte_storage release();
```

Releases the owned resource.

**Returns:** A value of type `byte_storage`.

**Source:** [include/common/writer.h:95](../../../include/common/writer.h#L95)
