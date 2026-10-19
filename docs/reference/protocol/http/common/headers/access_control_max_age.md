<a id="access-control-max-age"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/index.svg" alt="access_control_max_age">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_max_age/index-dark.svg" alt="access_control_max_age">
</a>

**Header:** [include/protocol/http/common/headers/access_control_max_age.h](../../../../../../include/protocol/http/common/headers/access_control_max_age.h)

|                                                    access-control-max-age | | Fetch Standard S3.3.3 HTTP responses                                      | | The "Access-Control-Max-Age" response header field indicates the number   | | of seconds for which the information provided by the                      | | Access-Control-Allow-Methods and Acc

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[access_control_max_age](#type-access-control-max-age)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::access_control_max_age

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-access-control-max-age"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_max_age

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_max_age { ... };
```

|                                                    access-control-max-age | | Fetch Standard S3.3.3 HTTP responses                                      | | The "Access-Control-Max-Age" response header field indicates the number   | | of seconds for which the information provided by the                      | | Access-Control-Allow-Methods and Acc

**Source:** [include/protocol/http/common/headers/access_control_max_age.h:90](../../../../../../include/protocol/http/common/headers/access_control_max_age.h#L90)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::access_control_max_age*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-max-age header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_max_age.h:95](../../../../../../include/protocol/http/common/headers/access_control_max_age.h#L95)
