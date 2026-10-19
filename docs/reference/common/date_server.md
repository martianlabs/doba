<a id="date-server-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/date_server/index-dark.svg">
    <img src="../../../resources/docs/reference/common/date_server/index.svg" alt="Index for date_server.h">
  </picture>
</h1>

**Header:** [include/common/date_server.h](../../../include/common/date_server.h)

This specification holds for a [cross-platform] date server.

**common**

- [date_server](#type-date-server) · *type*

**common::date_server**

- [date_server(date_server)](#constructor-date-server) · *constructor*
- [date_server(date_server)](#constructor-date-server-2) · *constructor*
- [~date_server](#destructor-destructor-date-server) · *destructor*
- [operator=(date_server)](#operator-operator) · *operator*
- [operator=(date_server)](#operator-operator-2) · *operator*
- [get](#method-get) · *method*
- [start](#method-start) · *method*
- [stop](#method-stop) · *method*
- [current](#method-current) · *method*

<a id="type-date-server"></a>

## date_server

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class date_server { ... };
```

This specification holds for a [cross-platform] date server.

**Source:** [include/common/date_server.h:47](../../../include/common/date_server.h#L47)

<a id="constructor-date-server"></a>

## date_server

🟠 **CONSTRUCTOR** · *common::date_server*

```cpp
date_server(const date_server&) = delete;
```

Disallows this form of common::date_server construction.

**Parameters:**

- **date_server:** Date server value to copy or move. Declaration: `const date_server&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/date_server.h:57](../../../include/common/date_server.h#L57)

<a id="constructor-date-server-2"></a>

## date_server

🟠 **CONSTRUCTOR** · *common::date_server*

```cpp
date_server(date_server&&) noexcept = delete;
```

Disallows this form of common::date_server construction.

**Parameters:**

- **date_server:** Date server value to copy or move. Declaration: `date_server&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/date_server.h:58](../../../include/common/date_server.h#L58)

<a id="destructor-destructor-date-server"></a>

## ~date_server

🟠 **DESTRUCTOR** · *common::date_server*

```cpp
~date_server();
```

Destroys the date_server object.

**Source:** [include/common/date_server.h:59](../../../include/common/date_server.h#L59)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *common::date_server*

```cpp
date_server& operator=(const date_server&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **date_server:** Date server value to copy or move. Declaration: `const date_server&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/date_server.h:69](../../../include/common/date_server.h#L69)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *common::date_server*

```cpp
date_server& operator=(date_server&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **date_server:** Date server value to copy or move. Declaration: `date_server&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/common/date_server.h:70](../../../include/common/date_server.h#L70)

<a id="method-get"></a>

## get

🟠 **METHOD** · *common::date_server*

```cpp
static date_server& get();
```

Returns the requested value.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/common/date_server.h:74](../../../include/common/date_server.h#L74)

<a id="method-start"></a>

## start

🟠 **METHOD** · *common::date_server*

```cpp
void start();
```

Starts the date_server lifecycle.

**Returns:** No value (`void`).

**Source:** [include/common/date_server.h:81](../../../include/common/date_server.h#L81)

<a id="method-stop"></a>

## stop

🟠 **METHOD** · *common::date_server*

```cpp
void stop();
```

Stops the date_server lifecycle.

**Returns:** No value (`void`).

**Source:** [include/common/date_server.h:102](../../../include/common/date_server.h#L102)

<a id="method-current"></a>

## current

🟠 **METHOD** · *common::date_server*

```cpp
std::string_view current() const noexcept;
```

Returns the current value.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/date_server.h:112](../../../include/common/date_server.h#L112)
