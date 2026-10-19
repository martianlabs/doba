<a id="referer"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/referer/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/referer/title.svg" alt="referer">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/referer/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/referer/title-dark.svg" alt="referer">
</a>

**Header:** [include/protocol/http/common/headers/referer.h](../../../../../../include/protocol/http/common/headers/referer.h)

|                                                                   referer | | RFC 9110 S10.1.3 Referer                                                  | | The "Referer" header field allows a user agent to specify a URI reference | | for the resource from which the target URI was obtained. This enables     | | servers to generate back-links, perf

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[referer](#type-referer)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::referer

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-referer"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> referer

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class referer { ... };
```

|                                                                   referer | | RFC 9110 S10.1.3 Referer                                                  | | The "Referer" header field allows a user agent to specify a URI reference | | for the resource from which the target URI was obtained. This enables     | | servers to generate back-links, perf

**Source:** [include/protocol/http/common/headers/referer.h:118](../../../../../../include/protocol/http/common/headers/referer.h#L118)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::referer*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP referer header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/referer.h:123](../../../../../../include/protocol/http/common/headers/referer.h#L123)
