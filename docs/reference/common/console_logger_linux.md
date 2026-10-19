<a id="console-logger-linux-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/console_logger_linux/index-dark.svg">
    <img src="../../../resources/docs/reference/common/console_logger_linux/index.svg" alt="Index for console_logger_linux.h">
  </picture>
</h1>

**Header:** [include/common/console_logger_linux.h](../../../include/common/console_logger_linux.h)

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

**Source:** [include/common/console_logger_linux.h:45](../../../include/common/console_logger_linux.h#L45)

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

**Source:** [include/common/console_logger_linux.h:51](../../../include/common/console_logger_linux.h#L51)

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

**Source:** [include/common/console_logger_linux.h:54](../../../include/common/console_logger_linux.h#L54)

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

**Source:** [include/common/console_logger_linux.h:55](../../../include/common/console_logger_linux.h#L55)

<a id="destructor-destructor-console-logger"></a>

## ~console_logger

🟠 **DESTRUCTOR** · *common::console_logger*

```cpp
~console_logger() = default;
```

Destroys the console_logger object.

**Source:** [include/common/console_logger_linux.h:56](../../../include/common/console_logger_linux.h#L56)

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

**Source:** [include/common/console_logger_linux.h:60](../../../include/common/console_logger_linux.h#L60)

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

**Source:** [include/common/console_logger_linux.h:61](../../../include/common/console_logger_linux.h#L61)

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

**Source:** [include/common/console_logger_linux.h:65](../../../include/common/console_logger_linux.h#L65)

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

**Source:** [include/common/console_logger_linux.h:68](../../../include/common/console_logger_linux.h#L68)

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

**Source:** [include/common/console_logger_linux.h:75](../../../include/common/console_logger_linux.h#L75)

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

**Source:** [include/common/console_logger_linux.h:78](../../../include/common/console_logger_linux.h#L78)

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

**Source:** [include/common/console_logger_linux.h:85](../../../include/common/console_logger_linux.h#L85)

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

**Source:** [include/common/console_logger_linux.h:88](../../../include/common/console_logger_linux.h#L88)

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

**Source:** [include/common/console_logger_linux.h:95](../../../include/common/console_logger_linux.h#L95)

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

**Source:** [include/common/console_logger_linux.h:98](../../../include/common/console_logger_linux.h#L98)

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

**Source:** [include/common/console_logger_linux.h:105](../../../include/common/console_logger_linux.h#L105)

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

**Source:** [include/common/console_logger_linux.h:108](../../../include/common/console_logger_linux.h#L108)
