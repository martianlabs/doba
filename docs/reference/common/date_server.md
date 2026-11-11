<a id="date-server"></a>
<a href="../../../resources/docs/reference/common/date_server/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/date_server/title.svg" alt="date_server">
</a>
<a href="../../../resources/docs/reference/common/date_server/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/date_server/title-dark.svg" alt="date_server">
</a>

**Header:** [include/common/date_server.h](../../../include/common/date_server.h)

This specification holds for a [cross-platform] date server.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[date_server](#type-date-server)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::date_server

[get](#method-get) ·
[start](#method-start) ·
[stop](#method-stop) ·
[current](#method-current) ·
[format](#method-format)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-date-server"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> date_server

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class date_server { ... };
```

This specification holds for a [cross-platform] date server.

**Source:** [include/common/date_server.h](../../../include/common/date_server.h)

<a id="method-get"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get

**METHOD** · *common::date_server*

```cpp
static date_server& get();
```

Return the process-wide singleton. Its first construction captures a time
sample, but does not start the updater thread.

**Returns:** A reference to the shared date server.

**Source:** [include/common/date_server.h](../../../include/common/date_server.h)

<a id="method-start"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> start

**METHOD** · *common::date_server*

```cpp
void start();
```

Adds one owner of the shared date updater. The first owner refreshes the date
and starts its thread; additional owners share that updater.
Balance each successful `start()` with one `stop()`. Lifecycle calls are
serialized internally. Updates run approximately once a second, not on a
real-time deadline.

**Returns:** No value (`void`).

**Source:** [include/common/date_server.h](../../../include/common/date_server.h)

<a id="method-stop"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop

**METHOD** · *common::date_server*

```cpp
void stop();
```

Releases one owner. The last owner stops and joins the updater thread.
Calling it with no owners has no effect.

**Returns:** No value (`void`).

**Source:** [include/common/date_server.h](../../../include/common/date_server.h)

<a id="method-current"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> current

**METHOD** · *common::date_server*

```cpp
std::string_view current() const noexcept;
```

Returns the cached HTTP date in GMT. The view uses thread-local storage and
may be replaced by the next `current()` call on that thread. Copy it if it
must survive later calls. An unrepresentable date returns an empty view.
With no running updater, this remains the last cached sample until the next
`start()`; it does not read the system clock on every call.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/date_server.h](../../../include/common/date_server.h)

<a id="method-format"></a>

## format

**METHOD** · *common::date_server*

```cpp
static std::string format(std::time_t timestamp);
```

Formats a timestamp as an HTTP date in GMT. Returns an empty string if the
timestamp cannot be converted or its year does not fit four digits.
The date server does not need to be running.
The returned string owns its bytes; allocation failure may throw.
