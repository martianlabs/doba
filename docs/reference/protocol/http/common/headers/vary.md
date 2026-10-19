<a id="vary-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/vary/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/vary/index.svg" alt="Index for vary.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/vary.h](../../../../../../include/protocol/http/common/headers/vary.h)

|                                                                      vary | | RFC 9110 S12.5.5 Vary                                                     | | The "Vary" response header field describes which parts of the request,    | | aside from the method and target URI, might have influenced the origin    | | server's process for selecting the r

**protocol::http::headers**

- [vary](#type-vary) · *type*

**protocol::http::headers::vary**

- [check](#method-check) · *method*

<a id="type-vary"></a>

## vary

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class vary { ... };
```

|                                                                      vary | | RFC 9110 S12.5.5 Vary                                                     | | The "Vary" response header field describes which parts of the request,    | | aside from the method and target URI, might have influenced the origin    | | server's process for selecting the r

**Source:** [include/protocol/http/common/headers/vary.h:115](../../../../../../include/protocol/http/common/headers/vary.h#L115)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::vary*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP vary header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/vary.h:120](../../../../../../include/protocol/http/common/headers/vary.h#L120)
