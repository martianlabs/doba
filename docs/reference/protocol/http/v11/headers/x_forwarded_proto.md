<a id="x-forwarded-proto-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/index.svg" alt="Index for x_forwarded_proto.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/index-dark.svg" alt="Index for x_forwarded_proto.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/headers/x_forwarded_proto.h](../../../../../../include/protocol/http/v11/headers/x_forwarded_proto.h)

De-facto X-Forwarded-Proto The "X-Forwarded-Proto" header field is a de-facto request header used by reverse proxies to disclose the protocol (scheme) used by the client to communicate with the proxy (typically "http" or "https").

**protocol::http::v11::headers**

- [x_forwarded_proto](#type-x-forwarded-proto) · *type*

**protocol::http::v11::headers::x_forwarded_proto**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-x-forwarded-proto"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> x_forwarded_proto

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class x_forwarded_proto { ... };
```

De-facto X-Forwarded-Proto The "X-Forwarded-Proto" header field is a de-facto request header used by reverse proxies to disclose the protocol (scheme) used by the client to communicate with the proxy (typically "http" or "https").

**Source:** [include/protocol/http/v11/headers/x_forwarded_proto.h:63](../../../../../../include/protocol/http/v11/headers/x_forwarded_proto.h#L63)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::x_forwarded_proto*

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Checks whether a value follows the HTTP x-forwarded-proto header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_token_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/x_forwarded_proto.h:68](../../../../../../include/protocol/http/v11/headers/x_forwarded_proto.h#L68)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::x_forwarded_proto*

```cpp
static constexpr verdict interpret(const parsed_token_list&, v11::connection&,
                                     const policies&);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **parsed_token_list:** Parsed token list value. Declaration: `const parsed_token_list&`.
- **connection:** Connection being processed. Declaration: `v11::connection&`.
- **policies:** Configuration policies for this component. Declaration: `const policies&`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/x_forwarded_proto.h:80](../../../../../../include/protocol/http/v11/headers/x_forwarded_proto.h#L80)
