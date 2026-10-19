<a id="console-logger"></a>
<a href="../../../resources/docs/reference/common/console_logger/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/console_logger/index.svg" alt="console_logger">
</a>
<a href="../../../resources/docs/reference/common/console_logger/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/console_logger/index-dark.svg" alt="console_logger">
</a>

**Header:** [include/common/console_logger.h](../../../include/common/console_logger.h)

Provides the console logger options type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[console_log_level](#enum-console-log-level) ·
[console_log_color](#enum-console-log-color) ·
[console_logger_options](#type-console-logger-options)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::console_logger_options

[show_name](#field-show-name) ·
[show_timestamp](#field-show-timestamp) ·
[show_function](#field-show-function) ·
[show_line](#field-show-line)

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="enum-console-log-level"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> console_log_level

**ENUM** · *common*

```cpp
enum class console_log_level { kDebug, kInfo, kWarning, kError, kCritical };
```

Enumerates console log level values.

**Values:**

- **kDebug:** Debug. Declaration: `kDebug`.
- **kInfo:** Info. Declaration: `kInfo`.
- **kWarning:** Warning. Declaration: `kWarning`.
- **kError:** Error. Declaration: `kError`.
- **kCritical:** Critical. Declaration: `kCritical`.

**Source:** [include/common/console_logger.h:36](../../../include/common/console_logger.h#L36)

<a id="enum-console-log-color"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> console_log_color

**ENUM** · *common*

```cpp
enum class console_log_color {
  kDefault,
  kBlack,
  kRed,
  kGreen,
  kYellow,
  kBlue,
  kMagenta,
  kCyan,
  kWhite
};
```

Enumerates console log color values.

**Values:**

- **kDefault:** Default. Declaration: `kDefault`.
- **kBlack:** Black. Declaration: `kBlack`.
- **kRed:** Red. Declaration: `kRed`.
- **kGreen:** Green. Declaration: `kGreen`.
- **kYellow:** Yellow. Declaration: `kYellow`.
- **kBlue:** Blue. Declaration: `kBlue`.
- **kMagenta:** Magenta. Declaration: `kMagenta`.
- **kCyan:** Cyan. Declaration: `kCyan`.
- **kWhite:** White. Declaration: `kWhite`.

**Source:** [include/common/console_logger.h:43](../../../include/common/console_logger.h#L43)

<a id="type-console-logger-options"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> console_logger_options

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct console_logger_options { ... };
```

Provides the console logger options type.

**Source:** [include/common/console_logger.h:60](../../../include/common/console_logger.h#L60)

<a id="field-show-name"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> show_name

**FIELD** · *common::console_logger_options*

```cpp
bool show_name{true};
```

Stores the show name value.

**Value:** `{true}`

**Source:** [include/common/console_logger.h:61](../../../include/common/console_logger.h#L61)

<a id="field-show-timestamp"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> show_timestamp

**FIELD** · *common::console_logger_options*

```cpp
bool show_timestamp{true};
```

Stores the show timestamp value.

**Value:** `{true}`

**Source:** [include/common/console_logger.h:62](../../../include/common/console_logger.h#L62)

<a id="field-show-function"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> show_function

**FIELD** · *common::console_logger_options*

```cpp
bool show_function{true};
```

Stores the show function value.

**Value:** `{true}`

**Source:** [include/common/console_logger.h:63](../../../include/common/console_logger.h#L63)

<a id="field-show-line"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> show_line

**FIELD** · *common::console_logger_options*

```cpp
bool show_line{true};
```

Stores the show line value.

**Value:** `{true}`

**Source:** [include/common/console_logger.h:64](../../../include/common/console_logger.h#L64)
