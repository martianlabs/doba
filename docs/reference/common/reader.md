<a id="reader-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/reader/index-dark.svg">
    <img src="../../../resources/docs/reference/common/reader/index.svg" alt="Index for reader.h">
  </picture>
</h1>

**Header:** [include/common/reader.h](../../../include/common/reader.h)

This class provides a reader interface for reading bytes from a byte_storage backend. It supports reading into a span of bytes, fetching individual bytes, and checking the state of the reader (e.g., whether it has reached the end of the storage or encountered an error). It can also read all remaining bytes into a string.

**common**

- [reader](#type-reader) · *type*

**common::reader**

- [reader()](#constructor-reader) · *constructor*
- [reader(storage)](#constructor-reader-2) · *constructor*
- [reader(file)](#constructor-reader-3) · *constructor*
- [reader(reader)](#constructor-reader-4) · *constructor*
- [reader(in)](#constructor-reader-5) · *constructor*
- [operator=(reader)](#operator-operator) · *operator*
- [operator=(in)](#operator-operator-2) · *operator*
- [borrowed](#method-borrowed) · *method*
- [rebind_borrowed](#method-rebind-borrowed) · *method*
- [read](#method-read) · *method*
- [fetch](#method-fetch) · *method*
- [eof](#method-eof) · *method*
- [exhausted](#method-exhausted) · *method*
- [failed](#method-failed) · *method*
- [ok](#method-ok) · *method*
- [size](#method-size) · *method*
- [read_all](#method-read-all) · *method*

<a id="type-reader"></a>

## reader

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class reader { ... };
```

This class provides a reader interface for reading bytes from a byte_storage backend. It supports reading into a span of bytes, fetching individual bytes, and checking the state of the reader (e.g., whether it has reached the end of the storage or encountered an error). It can also read all remaining bytes into a string.

**Source:** [include/common/reader.h:51](../../../include/common/reader.h#L51)

<a id="constructor-reader"></a>

## reader

🟠 **CONSTRUCTOR** · *common::reader*

```cpp
reader() = default;
```

Constructs a reader value.

**Source:** [include/common/reader.h:56](../../../include/common/reader.h#L56)

<a id="constructor-reader-2"></a>

## reader

🟠 **CONSTRUCTOR** · *common::reader*

```cpp
explicit reader(byte_storage storage);
```

Constructs a reader value.

**Parameters:**

- **storage:** Initial value supplied to the constructor. Declaration: `byte_storage storage`.

**Source:** [include/common/reader.h:57](../../../include/common/reader.h#L57)

<a id="constructor-reader-3"></a>

## reader

🟠 **CONSTRUCTOR** · *common::reader*

```cpp
explicit reader(filesystem_file file);
```

Constructs a reader value.

**Parameters:**

- **file:** File used by the operation. Declaration: `filesystem_file file`.

**Source:** [include/common/reader.h:58](../../../include/common/reader.h#L58)

<a id="constructor-reader-4"></a>

## reader

🟠 **CONSTRUCTOR** · *common::reader*

```cpp
reader(const reader&) = delete;
```

Disallows this form of common::reader construction.

**Parameters:**

- **reader:** Reader to use or move into this object. Declaration: `const reader&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/reader.h:59](../../../include/common/reader.h#L59)

<a id="constructor-reader-5"></a>

## reader

🟠 **CONSTRUCTOR** · *common::reader*

```cpp
reader(reader&& in) noexcept;
```

Constructs a reader value.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `reader&& in`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:60](../../../include/common/reader.h#L60)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *common::reader*

```cpp
reader& operator=(const reader&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **reader:** Reader to use or move into this object. Declaration: `const reader&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/reader.h:76](../../../include/common/reader.h#L76)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *common::reader*

```cpp
reader& operator=(reader&& in) noexcept;
```

Implements operator= for reader.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `reader&& in`.

**Returns:** A reference to the updated object.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:77](../../../include/common/reader.h#L77)

<a id="method-borrowed"></a>

## borrowed

🟠 **METHOD** · *common::reader*

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

## rebind_borrowed

🟠 **METHOD** · *common::reader*

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

## read

🟠 **METHOD** · *common::reader*

```cpp
std::size_t read(std::span<std::byte> output);
```

Reads data.

**Parameters:**

- **output:** Destination or output callback, as specified by its type. Declaration: `std::span<std::byte> output`.

**Returns:** The number of bytes processed.

**Source:** [include/common/reader.h:108](../../../include/common/reader.h#L108)

<a id="method-fetch"></a>

## fetch

🟠 **METHOD** · *common::reader*

```cpp
bool fetch(std::byte& byte);
```

Fetches the requested value.

**Parameters:**

- **byte:** Byte to read or write. Declaration: `std::byte& byte`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/reader.h:131](../../../include/common/reader.h#L131)

<a id="method-eof"></a>

## eof

🟠 **METHOD** · *common::reader*

```cpp
[[nodiscard]] bool eof() const noexcept;
```

Reports whether the eof condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:145](../../../include/common/reader.h#L145)

<a id="method-exhausted"></a>

## exhausted

🟠 **METHOD** · *common::reader*

```cpp
[[nodiscard]] bool exhausted() const noexcept;
```

Reports whether the exhausted condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:153](../../../include/common/reader.h#L153)

<a id="method-failed"></a>

## failed

🟠 **METHOD** · *common::reader*

```cpp
[[nodiscard]] bool failed() const noexcept;
```

Reports whether the failed condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:157](../../../include/common/reader.h#L157)

<a id="method-ok"></a>

## ok

🟠 **METHOD** · *common::reader*

```cpp
[[nodiscard]] bool ok() const noexcept;
```

Reports whether the ok condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:161](../../../include/common/reader.h#L161)

<a id="method-size"></a>

## size

🟠 **METHOD** · *common::reader*

```cpp
[[nodiscard]] std::optional<std::size_t> size() const noexcept;
```

Returns the current size.

**Returns:** A `std::optional<std::size_t>` containing the result when present; otherwise `std::nullopt`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/reader.h:168](../../../include/common/reader.h#L168)

<a id="method-read-all"></a>

## read_all

🟠 **METHOD** · *common::reader*

```cpp
std::size_t read_all(std::string& out);
```

Reads all.

**Parameters:**

- **out:** Output parameter that receives the result. Declaration: `std::string& out`.

**Returns:** The number of bytes processed.

**Source:** [include/common/reader.h:176](../../../include/common/reader.h#L176)
