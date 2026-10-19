<a id="writer-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/writer/index-dark.svg">
    <img src="../../../resources/docs/reference/common/writer/index.svg" alt="Index for writer.h">
  </picture>
</h1>

**Header:** [include/common/writer.h](../../../include/common/writer.h)

This class provides a writer interface for writing bytes to a byte_storage backend. It supports writing from a pointer, a string_view, or a span of bytes. It also provides methods to finish the writing process and check the state of the writer.

**common**

- [writer](#type-writer) · *type*

**common::writer**

- [writer()](#constructor-writer) · *constructor*
- [writer(opts)](#constructor-writer-2) · *constructor*
- [writer(writer)](#constructor-writer-3) · *constructor*
- [writer(writer)](#constructor-writer-4) · *constructor*
- [operator=(writer)](#operator-operator) · *operator*
- [operator=(writer)](#operator-operator-2) · *operator*
- [write(ptr, len)](#method-write) · *method*
- [write(bytes)](#method-write-2) · *method*
- [write(bytes)](#method-write-3) · *method*
- [finish](#method-finish) · *method*
- [ok](#method-ok) · *method*
- [release](#method-release) · *method*

<a id="type-writer"></a>

## writer

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class writer { ... };
```

This class provides a writer interface for writing bytes to a byte_storage backend. It supports writing from a pointer, a string_view, or a span of bytes. It also provides methods to finish the writing process and check the state of the writer.

**Source:** [include/common/writer.h:46](../../../include/common/writer.h#L46)

<a id="constructor-writer"></a>

## writer

🟠 **CONSTRUCTOR** · *common::writer*

```cpp
writer() = default;
```

Constructs a writer value.

**Source:** [include/common/writer.h:51](../../../include/common/writer.h#L51)

<a id="constructor-writer-2"></a>

## writer

🟠 **CONSTRUCTOR** · *common::writer*

```cpp
explicit writer(byte_storage_options opts);
```

Constructs a writer value.

**Parameters:**

- **opts:** Storage options; defaults to their default values. Declaration: `byte_storage_options opts`.

**Source:** [include/common/writer.h:52](../../../include/common/writer.h#L52)

<a id="constructor-writer-3"></a>

## writer

🟠 **CONSTRUCTOR** · *common::writer*

```cpp
writer(const writer&) = delete;
```

Disallows this form of common::writer construction.

**Parameters:**

- **writer:** Writer to use or move into this object. Declaration: `const writer&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/writer.h:53](../../../include/common/writer.h#L53)

<a id="constructor-writer-4"></a>

## writer

🟠 **CONSTRUCTOR** · *common::writer*

```cpp
writer(writer&&) noexcept = default;
```

Constructs a writer value.

**Parameters:**

- **writer:** Writer to use or move into this object. Declaration: `writer&&`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/writer.h:54](../../../include/common/writer.h#L54)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *common::writer*

```cpp
writer& operator=(const writer&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **writer:** Writer to use or move into this object. Declaration: `const writer&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/writer.h:58](../../../include/common/writer.h#L58)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *common::writer*

```cpp
writer& operator=(writer&&) noexcept = default;
```

Uses the compiler-generated operator= operation.

**Parameters:**

- **writer:** Writer to use or move into this object. Declaration: `writer&&`.

**Returns:** A reference to the updated object.

**Errors:** Declared `noexcept`.

**Source:** [include/common/writer.h:59](../../../include/common/writer.h#L59)

<a id="method-write"></a>

## write

🟠 **METHOD** · *common::writer*

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

## write

🟠 **METHOD** · *common::writer*

```cpp
bool write(std::string_view bytes);
```

Writes data.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::string_view bytes`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/writer.h:71](../../../include/common/writer.h#L71)

<a id="method-write-3"></a>

## write

🟠 **METHOD** · *common::writer*

```cpp
bool write(std::span<const std::byte> bytes);
```

Writes data.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::span<const std::byte> bytes`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/common/writer.h:77](../../../include/common/writer.h#L77)

<a id="method-finish"></a>

## finish

🟠 **METHOD** · *common::writer*

```cpp
void finish(std::size_t bytes);
```

Finalizes the buffered data or output stream.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::size_t bytes`.

**Returns:** No value (`void`).

**Source:** [include/common/writer.h:83](../../../include/common/writer.h#L83)

<a id="method-ok"></a>

## ok

🟠 **METHOD** · *common::writer*

```cpp
[[nodiscard]] bool ok() const noexcept;
```

Reports whether the ok condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/writer.h:91](../../../include/common/writer.h#L91)

<a id="method-release"></a>

## release

🟠 **METHOD** · *common::writer*

```cpp
[[nodiscard]] byte_storage release();
```

Releases the owned resource.

**Returns:** A value of type `byte_storage`.

**Source:** [include/common/writer.h:95](../../../include/common/writer.h#L95)
