<a id="reader"></a>
<a href="../../../resources/docs/reference/common/reader/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/reader/title.svg" alt="reader">
</a>
<a href="../../../resources/docs/reference/common/reader/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/reader/title-dark.svg" alt="reader">
</a>

**Header:** [include/common/reader.h](../../../include/common/reader.h)

This class provides a reader interface for reading bytes from a byte_storage backend. It supports reading into a span of bytes, fetching individual bytes, and checking the state of the reader (e.g., whether it has reached the end of the storage or encountered an error). It can also read all remaining bytes into a string.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[reader](#type-reader)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::reader

[borrowed](#method-borrowed) ·
[shared](#method-shared) ·
[rebind_borrowed](#method-rebind-borrowed) ·
[read](#method-read) ·
[fetch](#method-fetch) ·
[eof](#method-eof) ·
[exhausted](#method-exhausted) ·
[failed](#method-failed) ·
[ok](#method-ok) ·
[size](#method-size) ·
[read_all](#method-read-all)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-reader"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reader

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class reader { ... };
```

This class provides a reader interface for reading bytes from a byte_storage backend. It supports reading into a span of bytes, fetching individual bytes, and checking the state of the reader (e.g., whether it has reached the end of the storage or encountered an error). It can also read all remaining bytes into a string.

**Source:** [include/common/reader.h:53](../../../include/common/reader.h#L53)

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

**Source:** [include/common/reader.h:102](../../../include/common/reader.h#L102)

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

**Source:** [include/common/reader.h:114](../../../include/common/reader.h#L114)

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

**Source:** [include/common/reader.h:121](../../../include/common/reader.h#L121)

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

**Source:** [include/common/reader.h:144](../../../include/common/reader.h#L144)

<a id="method-eof"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> eof

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool eof() const noexcept;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:158](../../../include/common/reader.h#L158)

<a id="method-exhausted"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> exhausted

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool exhausted() const noexcept;
```

Reports whether the exhausted condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:161](../../../include/common/reader.h#L161)

<a id="method-failed"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> failed

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool failed() const noexcept;
```

Reports whether the failed condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:125](../../../include/common/reader.h#L125)

<a id="method-ok"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ok

**METHOD** · *common::reader*

```cpp
[[nodiscard]] bool ok() const noexcept;
```

Reports whether the ok condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:138](../../../include/common/reader.h#L138)

<a id="method-size"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> size

**METHOD** · *common::reader*

```cpp
[[nodiscard]] std::optional<std::size_t> size() const noexcept;
```

Returns the current size.

**Returns:** A `std::optional<std::size_t>` containing the result when present; otherwise `std::nullopt`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:129](../../../include/common/reader.h#L129)

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

**Source:** [include/common/reader.h:189](../../../include/common/reader.h#L189)

<a id="method-shared"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> shared

```cpp
static reader shared(std::shared_ptr<const void> owner,
                     std::span<const std::byte> source);
```

**Parameters:** owner retains stable immutable storage; source must refer into it.
Each reader has an independent cursor. Bytes must be immutable through all aliases.

**Returns:** An owning reader moved safely through the existing transport pipeline.

**Errors:** A null owner throws std::invalid_argument. rebind_borrowed() does not
rebind an owning shared reader. See the [cache example](../../../examples/common/cache/README.md).
