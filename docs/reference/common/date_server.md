<a id="date-server-h"></a>
<h1>
  <a href="../../../resources/docs/reference/common/date_server/index.svg#gh-light-mode-only">
    <img src="../../../resources/docs/reference/common/date_server/index.svg" alt="Index for date_server.h">
  </a>
  <a href="../../../resources/docs/reference/common/date_server/index-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/docs/reference/common/date_server/index-dark.svg" alt="Index for date_server.h">
  </a>
</h1>

**Header:** [include/common/date_server.h](../../../include/common/date_server.h)

This specification holds for a [cross-platform] date server.

**common**

- [date_server](#type-date-server) · *type*

**common::date_server**

- [get](#method-get) · *method*
- [start](#method-start) · *method*
- [stop](#method-stop) · *method*
- [current](#method-current) · *method*

<a id="type-date-server"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> date_server

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class date_server { ... };
```

This specification holds for a [cross-platform] date server.

**Source:** [include/common/date_server.h:47](../../../include/common/date_server.h#L47)

<a id="method-get"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get

**METHOD** · *common::date_server*

```cpp
static date_server& get();
```

Returns the requested value.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/common/date_server.h:74](../../../include/common/date_server.h#L74)

<a id="method-start"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> start

**METHOD** · *common::date_server*

```cpp
void start();
```

Starts the date_server lifecycle.

**Returns:** No value (`void`).

**Source:** [include/common/date_server.h:81](../../../include/common/date_server.h#L81)

<a id="method-stop"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> stop

**METHOD** · *common::date_server*

```cpp
void stop();
```

Stops the date_server lifecycle.

**Returns:** No value (`void`).

**Source:** [include/common/date_server.h:102](../../../include/common/date_server.h#L102)

<a id="method-current"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> current

**METHOD** · *common::date_server*

```cpp
std::string_view current() const noexcept;
```

Returns the current value.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/date_server.h:112](../../../include/common/date_server.h#L112)
