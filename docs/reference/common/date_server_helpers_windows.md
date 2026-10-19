<a id="date-server-helpers-windows"></a>
<a href="../../../resources/docs/reference/common/date_server_helpers_windows/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/date_server_helpers_windows/title.svg" alt="date_server_helpers_windows">
</a>
<a href="../../../resources/docs/reference/common/date_server_helpers_windows/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/date_server_helpers_windows/title-dark.svg" alt="date_server_helpers_windows">
</a>

**Header:** [include/common/date_server_helpers_windows.h](../../../include/common/date_server_helpers_windows.h)

This header declares the types and operations listed below.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[gm_time](#function-gm-time)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="function-gm-time"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> gm_time

**FUNCTION** · *common*

```cpp
static errno_t gm_time(struct tm* const gmt, time_t const* const now);
```

Converts a time value to UTC calendar fields.

**Parameters:**

- **gmt:** Output UTC calendar structure. Declaration: `struct tm* const gmt`.
- **now:** Input timestamp. Declaration: `time_t const* const now`.

**Returns:** A value of type `errno_t`.

**Source:** [include/common/date_server_helpers_windows.h:36](../../../include/common/date_server_helpers_windows.h#L36)
