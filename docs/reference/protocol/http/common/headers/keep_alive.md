<a id="keep-alive-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/keep_alive/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/keep_alive/index.svg" alt="Index for keep_alive.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/keep_alive.h](../../../../../../include/protocol/http/common/headers/keep_alive.h)

|                                                                keep-alive | | RFC 2068 S19.7.1.1 The Keep-Alive Header                                  | | The "Keep-Alive" header field is a historical HTTP/1.0 extension used     | | with explicitly negotiated persistent connections. It can carry optional  | | connection persistence parameters wh

**protocol::http::headers**

- [x_keep_alive](#type-x-keep-alive) · *type*

**protocol::http::headers::x_keep_alive**

- [check](#method-check) · *method*

<a id="type-x-keep-alive"></a>

## x_keep_alive

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class x_keep_alive { ... };
```

|                                                                keep-alive | | RFC 2068 S19.7.1.1 The Keep-Alive Header                                  | | The "Keep-Alive" header field is a historical HTTP/1.0 extension used     | | with explicitly negotiated persistent connections. It can carry optional  | | connection persistence parameters wh

**Source:** [include/protocol/http/common/headers/keep_alive.h:102](../../../../../../include/protocol/http/common/headers/keep_alive.h#L102)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::x_keep_alive*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP keep-alive header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/keep_alive.h:107](../../../../../../include/protocol/http/common/headers/keep_alive.h#L107)
