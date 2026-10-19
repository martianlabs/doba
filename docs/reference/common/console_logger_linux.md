<a id="console-logger-linux"></a>
<a href="../../../resources/docs/reference/common/console_logger_linux/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/console_logger_linux/index.svg" alt="console_logger_linux">
</a>
<a href="../../../resources/docs/reference/common/console_logger_linux/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/console_logger_linux/index-dark.svg" alt="console_logger_linux">
</a>

**Header:** [include/common/console_logger_linux.h](../../../include/common/console_logger_linux.h)

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

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-console-logger"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> console_logger

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class console_logger { ... };
```

Provides the console logger type.

**Source:** [include/common/console_logger_linux.h:45](../../../include/common/console_logger_linux.h#L45)

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

**Source:** [include/common/console_logger_linux.h:65](../../../include/common/console_logger_linux.h#L65)

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

**Source:** [include/common/console_logger_linux.h:68](../../../include/common/console_logger_linux.h#L68)

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

**Source:** [include/common/console_logger_linux.h:75](../../../include/common/console_logger_linux.h#L75)

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

**Source:** [include/common/console_logger_linux.h:78](../../../include/common/console_logger_linux.h#L78)

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

**Source:** [include/common/console_logger_linux.h:85](../../../include/common/console_logger_linux.h#L85)

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

**Source:** [include/common/console_logger_linux.h:88](../../../include/common/console_logger_linux.h#L88)

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

**Source:** [include/common/console_logger_linux.h:95](../../../include/common/console_logger_linux.h#L95)

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

**Source:** [include/common/console_logger_linux.h:98](../../../include/common/console_logger_linux.h#L98)

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

**Source:** [include/common/console_logger_linux.h:105](../../../include/common/console_logger_linux.h#L105)

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

**Source:** [include/common/console_logger_linux.h:108](../../../include/common/console_logger_linux.h#L108)
