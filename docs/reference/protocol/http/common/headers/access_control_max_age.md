<a id="access-control-max-age-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/index.svg" alt="Index for access_control_max_age.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/access_control_max_age.h](../../../../../../include/protocol/http/common/headers/access_control_max_age.h)

|                                                    access-control-max-age | | Fetch Standard S3.3.3 HTTP responses                                      | | The "Access-Control-Max-Age" response header field indicates the number   | | of seconds for which the information provided by the                      | | Access-Control-Allow-Methods and Acc

**protocol::http::headers**

- [access_control_max_age](#type-access-control-max-age) · *type*

**protocol::http::headers::access_control_max_age**

- [check](#method-check) · *method*

<a id="type-access-control-max-age"></a>

## access_control_max_age

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_max_age { ... };
```

|                                                    access-control-max-age | | Fetch Standard S3.3.3 HTTP responses                                      | | The "Access-Control-Max-Age" response header field indicates the number   | | of seconds for which the information provided by the                      | | Access-Control-Allow-Methods and Acc

**Source:** [include/protocol/http/common/headers/access_control_max_age.h:90](../../../../../../include/protocol/http/common/headers/access_control_max_age.h#L90)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::access_control_max_age*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-max-age header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_max_age.h:95](../../../../../../include/protocol/http/common/headers/access_control_max_age.h#L95)
