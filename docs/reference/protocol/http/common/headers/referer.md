<a id="referer-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/referer/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/referer/index.svg" alt="Index for referer.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/referer.h](../../../../../../include/protocol/http/common/headers/referer.h)

|                                                                   referer | | RFC 9110 S10.1.3 Referer                                                  | | The "Referer" header field allows a user agent to specify a URI reference | | for the resource from which the target URI was obtained. This enables     | | servers to generate back-links, perf

**protocol::http::headers**

- [referer](#type-referer) · *type*

**protocol::http::headers::referer**

- [check](#method-check) · *method*

<a id="type-referer"></a>

## referer

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class referer { ... };
```

|                                                                   referer | | RFC 9110 S10.1.3 Referer                                                  | | The "Referer" header field allows a user agent to specify a URI reference | | for the resource from which the target URI was obtained. This enables     | | servers to generate back-links, perf

**Source:** [include/protocol/http/common/headers/referer.h:118](../../../../../../include/protocol/http/common/headers/referer.h#L118)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::referer*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP referer header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/referer.h:123](../../../../../../include/protocol/http/common/headers/referer.h#L123)
