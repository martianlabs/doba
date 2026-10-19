<a id="console-line-windows-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/console_line_windows/index-dark.svg">
    <img src="../../../resources/docs/reference/common/console_line_windows/index.svg" alt="Index for console_line_windows.h">
  </picture>
</h1>

**Header:** [include/common/console_line_windows.h](../../../include/common/console_line_windows.h)

Provides the line type.

**doba**

- [line](#type-line) · *type*

**line**

- [line(line)](#constructor-line) · *constructor*
- [line(in)](#constructor-line-2) · *constructor*
- [~line](#destructor-destructor-line) · *destructor*
- [operator=(line)](#operator-operator) · *operator*
- [operator=(line)](#operator-operator-2) · *operator*
- [operator<<(value)](#operator-operator-3) · *operator*
- [operator<<(value)](#operator-operator-4) · *operator*
- [operator<<(value)](#operator-operator-5) · *operator*
- [operator<<(value)](#operator-operator-6) · *operator*
- [operator<<&lt;Ty&gt;(value)](#operator-operator-7) · *operator*
- [operator<<(color)](#operator-operator-8) · *operator*

<a id="type-line"></a>

## line

🟠 **TYPE** · *doba*

**Declaration synopsis:**

```cpp
class line { ... };
```

Provides the line type.

**Source:** [include/common/console_line_windows.h:31](../../../include/common/console_line_windows.h#L31)

<a id="constructor-line"></a>

## line

🟠 **CONSTRUCTOR** · *line*

```cpp
line(const line&) = delete;
```

Disallows this form of line construction.

**Parameters:**

- **line:** Line value to copy, move, or write. Declaration: `const line&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/console_line_windows.h:39](../../../include/common/console_line_windows.h#L39)

<a id="constructor-line-2"></a>

## line

🟠 **CONSTRUCTOR** · *line*

```cpp
line(line&& in) noexcept;
```

Constructs a line value.

**Parameters:**

- **in:** Input value to move or inspect. Declaration: `line&& in`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/console_line_windows.h:40](../../../include/common/console_line_windows.h#L40)

<a id="destructor-destructor-line"></a>

## ~line

🟠 **DESTRUCTOR** · *line*

```cpp
~line();
```

Destroys the line object.

**Source:** [include/common/console_line_windows.h:47](../../../include/common/console_line_windows.h#L47)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *line*

```cpp
line& operator=(const line&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **line:** Line value to copy, move, or write. Declaration: `const line&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/console_line_windows.h:55](../../../include/common/console_line_windows.h#L55)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *line*

```cpp
line& operator=(line&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **line:** Line value to copy, move, or write. Declaration: `line&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/console_line_windows.h:56](../../../include/common/console_line_windows.h#L56)

<a id="operator-operator-3"></a>

## operator<<

🟠 **OPERATOR** · *line*

```cpp
line& operator<<(std::string_view value);
```

Implements operator<< for line.

**Parameters:**

- **value:** Value to set or process. Declaration: `std::string_view value`.

**Returns:** A reference to the updated object.

**Source:** [include/common/console_line_windows.h:57](../../../include/common/console_line_windows.h#L57)

<a id="operator-operator-4"></a>

## operator<<

🟠 **OPERATOR** · *line*

```cpp
line& operator<<(const char* value);
```

Implements operator<< for line.

**Parameters:**

- **value:** Value to set or process. Declaration: `const char* value`.

**Returns:** A reference to the updated object.

**Source:** [include/common/console_line_windows.h:62](../../../include/common/console_line_windows.h#L62)

<a id="operator-operator-5"></a>

## operator<<

🟠 **OPERATOR** · *line*

```cpp
line& operator<<(char value);
```

Implements operator<< for line.

**Parameters:**

- **value:** Value to set or process. Declaration: `char value`.

**Returns:** A reference to the updated object.

**Source:** [include/common/console_line_windows.h:65](../../../include/common/console_line_windows.h#L65)

<a id="operator-operator-6"></a>

## operator<<

🟠 **OPERATOR** · *line*

```cpp
line& operator<<(bool value);
```

Implements operator<< for line.

**Parameters:**

- **value:** Value to set or process. Declaration: `bool value`.

**Returns:** A reference to the updated object.

**Source:** [include/common/console_line_windows.h:70](../../../include/common/console_line_windows.h#L70)

<a id="operator-operator-7"></a>

## operator<<

🟠 **OPERATOR** · *line*

```cpp
template <typename Ty>
      requires(std::is_arithmetic_v<Ty> &&
               !std::is_same_v<std::remove_cv_t<Ty>, char> &&
               !std::is_same_v<std::remove_cv_t<Ty>, bool>)
    line& operator<<(Ty value);
```

Implements operator<< for line.

**Template parameters:**

- **Ty:** `typename Ty`.

**Parameters:**

- **value:** Value to set or process. Declaration: `Ty value`.

**Returns:** A reference to the updated object.

**Source:** [include/common/console_line_windows.h:77](../../../include/common/console_line_windows.h#L77)

<a id="operator-operator-8"></a>

## operator<<

🟠 **OPERATOR** · *line*

```cpp
line& operator<<(console_log_color color);
```

Implements operator<< for line.

**Parameters:**

- **color:** Color used for console output. Declaration: `console_log_color color`.

**Returns:** A reference to the updated object.

**Source:** [include/common/console_line_windows.h:86](../../../include/common/console_line_windows.h#L86)
