<a id="console-logger-windows-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/console_logger_windows/index-dark.svg">
    <img src="../../../resources/docs/reference/common/console_logger_windows/index.svg" alt="Index for console_logger_windows.h">
  </picture>
</h1>

**Header:** [include/common/console_logger_windows.h](../../../include/common/console_logger_windows.h)

Provides the console logger type.

**common**

- [console_logger](#type-console-logger) · *type*

**common::console_logger**

- [console_logger(name, options)](#constructor-console-logger) · *constructor*
- [console_logger(console_logger)](#constructor-console-logger-2) · *constructor*
- [console_logger(console_logger)](#constructor-console-logger-3) · *constructor*
- [~console_logger](#destructor-destructor-console-logger) · *destructor*
- [operator=(console_logger)](#operator-operator) · *operator*
- [operator=(console_logger)](#operator-operator-2) · *operator*
- [debug(source)](#method-debug) · *method*
- [debug(message, source)](#method-debug-2) · *method*
- [info(source)](#method-info) · *method*
- [info(message, source)](#method-info-2) · *method*
- [warning(source)](#method-warning) · *method*
- [warning(message, source)](#method-warning-2) · *method*
- [error(source)](#method-error) · *method*
- [error(message, source)](#method-error-2) · *method*
- [critical(source)](#method-critical) · *method*
- [critical(message, source)](#method-critical-2) · *method*

<a id="type-console-logger"></a>

## console_logger

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class console_logger { ... };
```

Provides the console logger type.

**Source:** [include/common/console_logger_windows.h:47](../../../include/common/console_logger_windows.h#L47)

<a id="constructor-console-logger"></a>

## console_logger

🟠 **CONSTRUCTOR** · *common::console_logger*

```cpp
explicit console_logger(std::string_view name,
                          console_logger_options options = {});
```

Constructs a console_logger value.

**Parameters:**

- **name:** Name to match or store. Declaration: `std::string_view name`.
- **options:** Initial value supplied to the constructor. Declaration: `console_logger_options options = {}`.

**Source:** [include/common/console_logger_windows.h:53](../../../include/common/console_logger_windows.h#L53)

<a id="constructor-console-logger-2"></a>

## console_logger

🟠 **CONSTRUCTOR** · *common::console_logger*

```cpp
console_logger(const console_logger&) = delete;
```

Disallows this form of common::console_logger construction.

**Parameters:**

- **console_logger:** Source object to copy or move. Declaration: `const console_logger&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/console_logger_windows.h:56](../../../include/common/console_logger_windows.h#L56)

<a id="constructor-console-logger-3"></a>

## console_logger

🟠 **CONSTRUCTOR** · *common::console_logger*

```cpp
console_logger(console_logger&&) noexcept = delete;
```

Disallows this form of common::console_logger construction.

**Parameters:**

- **console_logger:** Source object to copy or move. Declaration: `console_logger&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/console_logger_windows.h:57](../../../include/common/console_logger_windows.h#L57)

<a id="destructor-destructor-console-logger"></a>

## ~console_logger

🟠 **DESTRUCTOR** · *common::console_logger*

```cpp
~console_logger() = default;
```

Destroys the console_logger object.

**Source:** [include/common/console_logger_windows.h:58](../../../include/common/console_logger_windows.h#L58)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *common::console_logger*

```cpp
console_logger& operator=(const console_logger&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **console_logger:** Source object to copy or move. Declaration: `const console_logger&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/console_logger_windows.h:62](../../../include/common/console_logger_windows.h#L62)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *common::console_logger*

```cpp
console_logger& operator=(console_logger&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **console_logger:** Source object to copy or move. Declaration: `console_logger&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/console_logger_windows.h:63](../../../include/common/console_logger_windows.h#L63)

<a id="method-debug"></a>

## debug

🟠 **METHOD** · *common::console_logger*

```cpp
line debug(std::source_location source = std::source_location::current());
```

Writes a debug-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:67](../../../include/common/console_logger_windows.h#L67)

<a id="method-debug-2"></a>

## debug

🟠 **METHOD** · *common::console_logger*

```cpp
void debug(std::string_view message,
             std::source_location source = std::source_location::current());
```

Writes a debug-level log message.

**Parameters:**

- **message:** Message text to log or send. Declaration: `std::string_view message`.
- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** No value (`void`).

**Source:** [include/common/console_logger_windows.h:70](../../../include/common/console_logger_windows.h#L70)

<a id="method-info"></a>

## info

🟠 **METHOD** · *common::console_logger*

```cpp
line info(std::source_location source = std::source_location::current());
```

Writes an info-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:77](../../../include/common/console_logger_windows.h#L77)

<a id="method-info-2"></a>

## info

🟠 **METHOD** · *common::console_logger*

```cpp
void info(std::string_view message,
            std::source_location source = std::source_location::current());
```

Writes an info-level log message.

**Parameters:**

- **message:** Message text to log or send. Declaration: `std::string_view message`.
- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** No value (`void`).

**Source:** [include/common/console_logger_windows.h:80](../../../include/common/console_logger_windows.h#L80)

<a id="method-warning"></a>

## warning

🟠 **METHOD** · *common::console_logger*

```cpp
line warning(std::source_location source = std::source_location::current());
```

Writes a warning-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:87](../../../include/common/console_logger_windows.h#L87)

<a id="method-warning-2"></a>

## warning

🟠 **METHOD** · *common::console_logger*

```cpp
void warning(std::string_view message,
               std::source_location source = std::source_location::current());
```

Writes a warning-level log message.

**Parameters:**

- **message:** Message text to log or send. Declaration: `std::string_view message`.
- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** No value (`void`).

**Source:** [include/common/console_logger_windows.h:90](../../../include/common/console_logger_windows.h#L90)

<a id="method-error"></a>

## error

🟠 **METHOD** · *common::console_logger*

```cpp
line error(std::source_location source = std::source_location::current());
```

Writes an error-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:97](../../../include/common/console_logger_windows.h#L97)

<a id="method-error-2"></a>

## error

🟠 **METHOD** · *common::console_logger*

```cpp
void error(std::string_view message,
             std::source_location source = std::source_location::current());
```

Writes an error-level log message.

**Parameters:**

- **message:** Message text to log or send. Declaration: `std::string_view message`.
- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** No value (`void`).

**Source:** [include/common/console_logger_windows.h:100](../../../include/common/console_logger_windows.h#L100)

<a id="method-critical"></a>

## critical

🟠 **METHOD** · *common::console_logger*

```cpp
line critical(std::source_location source = std::source_location::current());
```

Writes a critical-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:107](../../../include/common/console_logger_windows.h#L107)

<a id="method-critical-2"></a>

## critical

🟠 **METHOD** · *common::console_logger*

```cpp
void critical(std::string_view message,
                std::source_location source = std::source_location::current());
```

Writes a critical-level log message.

**Parameters:**

- **message:** Message text to log or send. Declaration: `std::string_view message`.
- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** No value (`void`).

**Source:** [include/common/console_logger_windows.h:110](../../../include/common/console_logger_windows.h#L110)
