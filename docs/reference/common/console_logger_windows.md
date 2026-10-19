<a id="console-logger-windows"></a>
<a href="../../../resources/docs/reference/common/console_logger_windows/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/console_logger_windows/index.svg" alt="console_logger_windows">
</a>
<a href="../../../resources/docs/reference/common/console_logger_windows/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/console_logger_windows/index-dark.svg" alt="console_logger_windows">
</a>

**Header:** [include/common/console_logger_windows.h](../../../include/common/console_logger_windows.h)

Provides the console logger type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[console_logger](#type-console-logger)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::console_logger

[debug(source)](#method-debug) ·
[debug(message, source)](#method-debug-2) ·
[info(source)](#method-info) ·
[info(message, source)](#method-info-2) ·
[warning(source)](#method-warning) ·
[warning(message, source)](#method-warning-2) ·
[error(source)](#method-error) ·
[error(message, source)](#method-error-2) ·
[critical(source)](#method-critical) ·
[critical(message, source)](#method-critical-2)

<a id="type-console-logger"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> console_logger

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class console_logger { ... };
```

Provides the console logger type.

**Source:** [include/common/console_logger_windows.h:47](../../../include/common/console_logger_windows.h#L47)

<a id="method-debug"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> debug

**METHOD** · *common::console_logger*

```cpp
line debug(std::source_location source = std::source_location::current());
```

Writes a debug-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:67](../../../include/common/console_logger_windows.h#L67)

<a id="method-debug-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> debug

**METHOD** · *common::console_logger*

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

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> info

**METHOD** · *common::console_logger*

```cpp
line info(std::source_location source = std::source_location::current());
```

Writes an info-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:77](../../../include/common/console_logger_windows.h#L77)

<a id="method-info-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> info

**METHOD** · *common::console_logger*

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

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> warning

**METHOD** · *common::console_logger*

```cpp
line warning(std::source_location source = std::source_location::current());
```

Writes a warning-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:87](../../../include/common/console_logger_windows.h#L87)

<a id="method-warning-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> warning

**METHOD** · *common::console_logger*

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

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> error

**METHOD** · *common::console_logger*

```cpp
line error(std::source_location source = std::source_location::current());
```

Writes an error-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:97](../../../include/common/console_logger_windows.h#L97)

<a id="method-error-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> error

**METHOD** · *common::console_logger*

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

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> critical

**METHOD** · *common::console_logger*

```cpp
line critical(std::source_location source = std::source_location::current());
```

Writes a critical-level log message.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `std::source_location source = std::source_location::current()`.

**Returns:** A log line at the selected severity, ready for streamed content.

**Source:** [include/common/console_logger_windows.h:107](../../../include/common/console_logger_windows.h#L107)

<a id="method-critical-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> critical

**METHOD** · *common::console_logger*

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
