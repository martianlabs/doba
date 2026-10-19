<a id="reader"></a>
<a href="../../../resources/docs/reference/common/reader/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/reader/index.svg" alt="reader">
</a>
<a href="../../../resources/docs/reference/common/reader/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/reader/index-dark.svg" alt="reader">
</a>

**Header:** [include/common/reader.h](../../../include/common/reader.h)

This class provides a reader interface for reading bytes from a byte_storage backend. It supports reading into a span of bytes, fetching individual bytes, and checking the state of the reader (e.g., whether it has reached the end of the storage or encountered an error). It can also read all remaining bytes into a string.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[reader](#type-reader)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::reader

[borrowed](#method-borrowed) ·
[rebind_borrowed](#method-rebind-borrowed) ·
[read](#method-read) ·
[fetch](#method-fetch) ·
[eof](#method-eof) ·
[exhausted](#method-exhausted) ·
[failed](#method-failed) ·
[ok](#method-ok) ·
[size](#method-size) ·
[read_all](#method-read-all)

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-reader"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class reader { ... };
```

This class provides a reader interface for reading bytes from a byte_storage backend. It supports reading into a span of bytes, fetching individual bytes, and checking the state of the reader (e.g., whether it has reached the end of the storage or encountered an error). It can also read all remaining bytes into a string.

**Source:** [include/common/reader.h:51](../../../include/common/reader.h#L51)

<a id="method-borrowed"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> borrowed

**METHOD** · *common::reader*

```cpp
static reader borrowed(std::span<const std::byte> source) noexcept;
```

Creates a non-owning reader over stable caller storage. The referenced bytes must outlive this reader and every object to which it is moved.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::span<const std::byte> source`.

**Returns:** A `reader` configured for the selected body format.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:98](../../../include/common/reader.h#L98)

<a id="method-rebind-borrowed"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> rebind_borrowed

**METHOD** · *common::reader*

```cpp
void rebind_borrowed(std::span<const std::byte> source) noexcept;
```

Points borrowed byte views at the replacement buffer.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::span<const std::byte> source`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:101](../../../include/common/reader.h#L101)

<a id="method-read"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read

**METHOD** · *common::reader*

```cpp
std::size_t read(std::span<std::byte> output);
```

Reads data.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `std::span<std::byte> output`.

**Returns:** The number of bytes processed.

**Source:** [include/common/reader.h:108](../../../include/common/reader.h#L108)

<a id="method-fetch"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> fetch

**METHOD** · *common::reader*

```cpp
bool fetch(std::byte& byte);
```

Fetches the requested value.

**Parameters:**

- **byte:** Byte to read or write. Declaration: `std::byte& byte`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/reader.h:131](../../../include/common/reader.h#L131)

<a id="method-eof"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> eof

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool eof() const noexcept;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:145](../../../include/common/reader.h#L145)

<a id="method-exhausted"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> exhausted

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool exhausted() const noexcept;
```

Reports whether the exhausted condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:153](../../../include/common/reader.h#L153)

<a id="method-failed"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> failed

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool failed() const noexcept;
```

Reports whether the failed condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:157](../../../include/common/reader.h#L157)

<a id="method-ok"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ok

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool ok() const noexcept;
```

Reports whether the ok condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:161](../../../include/common/reader.h#L161)

<a id="method-size"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> size

**METHOD** · *common::reader*

```cpp
[[nodiscard]] std::optional<std::size_t> size() const noexcept;
```

Returns the current size.

**Returns:** A `std::optional<std::size_t>` containing the result when present; otherwise `std::nullopt`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:168](../../../include/common/reader.h#L168)

<a id="method-read-all"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> read_all

**METHOD** · *common::reader*

```cpp
std::size_t read_all(std::string& out);
```

Reads all.

**Parameters:**

- **out:** Output parameter that receives the result. Declaration: `std::string& out`.

**Returns:** The number of bytes processed.

**Source:** [include/common/reader.h:176](../../../include/common/reader.h#L176)
