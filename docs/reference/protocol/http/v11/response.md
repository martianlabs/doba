<a id="response"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/response/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/response/title.svg" alt="response">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/response/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/response/title-dark.svg" alt="response">
</a>

**Header:** [include/protocol/http/v11/response.h](../../../../../include/protocol/http/v11/response.h)

Provides the http 1.1 response implementation.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[response](#type-response)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::response

[serialized_type](#alias-serialized-type) ·
[from(head_limit, body_threshold)](#method-from) ·
[from(head_limit, body_threshold, head, body)](#method-from-2) ·
[serialize](#method-serialize) ·
[add_header(k, v)](#method-add-header) ·
[add_header&lt;T&gt;(key, val)](#method-add-header-2) ·
[set_header(k, v)](#method-set-header) ·
[set_header&lt;T&gt;(key, val)](#method-set-header-2) ·
[has_header](#method-has-header) ·
[wants_connection_close](#method-wants-connection-close) ·
[is_continue_100](#method-is-continue-100) ·
[get_header(k)](#method-get-header) ·
[get_header(index)](#method-get-header-2) ·
[get_headers_length](#method-get-headers-length) ·
[remove_header](#method-remove-header) ·
[set_body(sv)](#method-set-body) ·
[set_body(source)](#method-set-body-2) ·
[set_body(writer)](#method-set-body-3) ·
[set_body&lt;T&gt;(val)](#method-set-body-4) ·
[set_body(source, length)](#method-set-body-5) ·
[clear_body](#method-clear-body) ·
[suppress_body](#method-suppress-body) ·
[continue_100](#method-continue-100) ·
[switching_protocols_101](#method-switching-protocols-101) ·
[ok_200](#method-ok-200) ·
[created_201](#method-created-201) ·
[accepted_202](#method-accepted-202) ·
[non_authoritative_info_203](#method-non-authoritative-info-203) ·
[no_content_204](#method-no-content-204) ·
[reset_content_205](#method-reset-content-205) ·
[partial_content_206](#method-partial-content-206) ·
[multiple_choices_300](#method-multiple-choices-300) ·
[moved_permanently_301](#method-moved-permanently-301) ·
[found_302](#method-found-302) ·
[see_other_303](#method-see-other-303) ·
[not_modified_304](#method-not-modified-304) ·
[use_proxy_305](#method-use-proxy-305) ·
[unused_306](#method-unused-306) ·
[temporary_redirect_307](#method-temporary-redirect-307) ·
[permanent_redirect_308](#method-permanent-redirect-308) ·
[bad_request_400](#method-bad-request-400) ·
[unauthorized_401](#method-unauthorized-401) ·
[payment_required_402](#method-payment-required-402) ·
[forbidden_403](#method-forbidden-403) ·
[not_found_404](#method-not-found-404) ·
[method_not_allowed_405](#method-method-not-allowed-405) ·
[not_acceptable_406](#method-not-acceptable-406) ·
[proxy_auth_required_407](#method-proxy-auth-required-407) ·
[request_timeout_408](#method-request-timeout-408) ·
[conflict_409](#method-conflict-409) ·
[gone_410](#method-gone-410) ·
[length_required_411](#method-length-required-411) ·
[precondition_failed_412](#method-precondition-failed-412) ·
[content_too_large_413](#method-content-too-large-413) ·
[uri_too_long_414](#method-uri-too-long-414) ·
[unsupported_media_type_415](#method-unsupported-media-type-415) ·
[range_not_satisfiable_416](#method-range-not-satisfiable-416) ·
[expectation_failed_417](#method-expectation-failed-417) ·
[unused_418](#method-unused-418) ·
[misdirected_request_421](#method-misdirected-request-421) ·
[unprocessable_content_422](#method-unprocessable-content-422) ·
[upgrade_required_426](#method-upgrade-required-426) ·
[request_header_fields_too_large_431](#method-request-header-fields-too-large-431) ·
[internal_server_error_500](#method-internal-server-error-500) ·
[not_implemented_501](#method-not-implemented-501) ·
[bad_gateway_502](#method-bad-gateway-502) ·
[service_unavailable_503](#method-service-unavailable-503) ·
[gateway_timeout_504](#method-gateway-timeout-504) ·
[http_version_not_supported_505](#method-http-version-not-supported-505)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-response"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> response

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
class response { ... };
```

Provides the http 1.1 response implementation.

**Source:** [include/protocol/http/v11/response.h:60](../../../../../include/protocol/http/v11/response.h#L60)

<a id="alias-serialized-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> serialized_type

**ALIAS** · *protocol::http::v11::response*

```cpp
using serialized_type = serialization_result;
```

Alias for `serialization_result`.

**Source:** [include/protocol/http/v11/response.h:65](../../../../../include/protocol/http/v11/response.h#L65)

<a id="method-from"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> from

**METHOD** · *protocol::http::v11::response*

```cpp
static response from(std::size_t head_limit, std::size_t body_threshold);
```

Constructs the value from the supplied input.

**Parameters:**

- **head_limit:** Maximum HTTP head size, in bytes. Declaration: `std::size_t head_limit`.
- **body_threshold:** Maximum in-memory body size, in bytes. Declaration: `std::size_t body_threshold`.

**Returns:** A value of type `response`.

**Source:** [include/protocol/http/v11/response.h:122](../../../../../include/protocol/http/v11/response.h#L122)

<a id="method-from-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> from

**METHOD** · *protocol::http::v11::response*

```cpp
static response from(std::size_t head_limit, std::size_t body_threshold,
                       std::string& head, std::string& body);
```

Constructs the value from the supplied input.

**Parameters:**

- **head_limit:** Maximum HTTP head size, in bytes. Declaration: `std::size_t head_limit`.
- **body_threshold:** Maximum in-memory body size, in bytes. Declaration: `std::size_t body_threshold`.
- **head:** HTTP head data or its output buffer. Declaration: `std::string& head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string& body`.

**Returns:** A value of type `response`.

**Source:** [include/protocol/http/v11/response.h:125](../../../../../include/protocol/http/v11/response.h#L125)

<a id="method-serialize"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> serialize

**METHOD** · *protocol::http::v11::response*

```cpp
[[nodiscard]] serialization_result serialize();
```

Borrows head and body until on_send copies them; moves the source.

**Returns:** A `serialization_result` describing the serialized output.

**Errors:** Directly throws `std::logic_error`, `std::out_of_range`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/response.h:168](../../../../../include/protocol/http/v11/response.h#L168)

<a id="method-add-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add_header

**METHOD** · *protocol::http::v11::response*

```cpp
response& add_header(std::string_view k, std::string_view v);
```

Adds header.

**Parameters:**

- **k:** Key to find or store. Declaration: `std::string_view k`.
- **v:** Text value to inspect. Declaration: `std::string_view v`.

**Returns:** A reference to this object, for chaining calls.

**Errors:** Directly throws `std::invalid_argument`, `std::out_of_range`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/response.h:239](../../../../../include/protocol/http/v11/response.h#L239)

<a id="method-add-header-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add_header

**METHOD** · *protocol::http::v11::response*

```cpp
template <typename T>
    requires std::is_arithmetic_v<T>
  response& add_header(std::string_view key, const T& val);
```

Adds header.

**Template parameters:**

- **T:** `typename T`.

**Parameters:**

- **key:** Key to find or store. Declaration: `std::string_view key`.
- **val:** Value to parse or store. Declaration: `const T& val`.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:280](../../../../../include/protocol/http/v11/response.h#L280)

<a id="method-set-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_header

**METHOD** · *protocol::http::v11::response*

```cpp
response& set_header(std::string_view k, std::string_view v);
```

Sets header.

**Parameters:**

- **k:** Key to find or store. Declaration: `std::string_view k`.
- **v:** Text value to inspect. Declaration: `std::string_view v`.

**Returns:** A reference to this object, for chaining calls.

**Errors:** Directly throws `std::invalid_argument`, `std::out_of_range`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/response.h:286](../../../../../include/protocol/http/v11/response.h#L286)

<a id="method-set-header-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_header

**METHOD** · *protocol::http::v11::response*

```cpp
template <typename T>
    requires std::is_arithmetic_v<T>
  response& set_header(std::string_view key, const T& val);
```

Sets header.

**Template parameters:**

- **T:** `typename T`.

**Parameters:**

- **key:** Key to find or store. Declaration: `std::string_view key`.
- **val:** Value to parse or store. Declaration: `const T& val`.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:326](../../../../../include/protocol/http/v11/response.h#L326)

<a id="method-has-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> has_header

**METHOD** · *protocol::http::v11::response*

```cpp
bool has_header(std::string_view k) const;
```

Returns true if a header whose name case-insensitively matches 'k' is present. Lets callers probe for a header without incurring the cost (or control flow) of catching the exception thrown by get_header.

**Parameters:**

- **k:** Key to find or store. Declaration: `std::string_view k`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/response.h:336](../../../../../include/protocol/http/v11/response.h#L336)

<a id="method-wants-connection-close"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> wants_connection_close

**METHOD** · *protocol::http::v11::response*

```cpp
bool wants_connection_close() const;
```

Reports whether the connection should close.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/response.h:344](../../../../../include/protocol/http/v11/response.h#L344)

<a id="method-is-continue-100"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_continue_100

**METHOD** · *protocol::http::v11::response*

```cpp
bool is_continue_100() const;
```

Sets the HTTP response status to 100 (is continue).

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/response.h:348](../../../../../include/protocol/http/v11/response.h#L348)

<a id="method-get-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_header

**METHOD** · *protocol::http::v11::response*

```cpp
std::pair<std::string, std::string> get_header(std::string_view k) const;
```

Returns the (key, value) pair for the first header whose name case-insensitively matches 'k'. Throws std::runtime_error if the header is not present. The returned strings are owned copies: they do NOT alias response storage and stay valid across later mutations.

**Parameters:**

- **k:** Key to find or store. Declaration: `std::string_view k`.

**Returns:** A value of type `std::pair<std::string, std::string>`.

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/response.h:357](../../../../../include/protocol/http/v11/response.h#L357)

<a id="method-get-header-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_header

**METHOD** · *protocol::http::v11::response*

```cpp
std::pair<std::string, std::string> get_header(std::size_t index) const;
```

Returns the (key, value) pair for the header at position 'index' (in insertion order). Throws std::out_of_range if 'index' is beyond the number of headers. The returned strings are owned copies (see above).

**Parameters:**

- **index:** Zero-based position of the requested element. Declaration: `std::size_t index`.

**Returns:** A value of type `std::pair<std::string, std::string>`.

**Errors:** Directly throws `std::out_of_range`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/response.h:371](../../../../../include/protocol/http/v11/response.h#L371)

<a id="method-get-headers-length"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_headers_length

**METHOD** · *protocol::http::v11::response*

```cpp
std::size_t get_headers_length() const;
```

Returns the number of headers currently stored. Enables safe indexed iteration via get_header(index) without relying on catching exceptions.

**Returns:** The get headers length count or size.

**Source:** [include/protocol/http/v11/response.h:401](../../../../../include/protocol/http/v11/response.h#L401)

<a id="method-remove-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> remove_header

**METHOD** · *protocol::http::v11::response*

```cpp
response& remove_header(std::string_view k);
```

Removes the first header whose name case-insensitively matches 'k' and compacts the header block. Removing an absent header is an intentional no-op (idempotent): unlike get_header, it does not throw when 'k' is not present.

**Parameters:**

- **k:** Key to find or store. Declaration: `std::string_view k`.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:426](../../../../../include/protocol/http/v11/response.h#L426)

<a id="method-set-body"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_body

**METHOD** · *protocol::http::v11::response*

```cpp
response& set_body(std::string_view sv);
```

Stores small payloads in response storage; larger ones use a raw writer.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A reference to this object, for chaining calls.

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/response.h:449](../../../../../include/protocol/http/v11/response.h#L449)

<a id="method-set-body-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_body

**METHOD** · *protocol::http::v11::response*

```cpp
response& set_body(body::reader* source);
```

Copies decoded payloads into response storage.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `body::reader* source`.

**Returns:** A reference to this object, for chaining calls.

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/response.h:472](../../../../../include/protocol/http/v11/response.h#L472)

<a id="method-set-body-3"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_body

**METHOD** · *protocol::http::v11::response*

```cpp
response& set_body(body::body_writer&& writer);
```

Adopts a caller-built body::body_writer (possibly already written to) as the response body, replacing any previously set body. Framing headers are emitted at serialization from the writer's mode and size.

**Parameters:**

- **writer:** Writer to use or move into this object. Declaration: `body::body_writer&& writer`.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:536](../../../../../include/protocol/http/v11/response.h#L536)

<a id="method-set-body-4"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_body

**METHOD** · *protocol::http::v11::response*

```cpp
template <typename T>
    requires std::is_arithmetic_v<T>
  response& set_body(const T& val);
```

Convenience overload for numeric types: converts the value to a string and stores it as the response body with deferred framing.

**Template parameters:**

- **T:** `typename T`.

**Parameters:**

- **val:** Value to parse or store. Declaration: `const T& val`.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:551](../../../../../include/protocol/http/v11/response.h#L551)

<a id="method-set-body-5"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> set_body

**METHOD** · *protocol::http::v11::response*

```cpp
response& set_body(common::reader&& source, std::size_t length);
```

Sets body.

**Parameters:**

- **source:** Source data or object to read from. Declaration: `common::reader&& source`.
- **length:** Length in elements or bytes. Declaration: `std::size_t length`.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:558](../../../../../include/protocol/http/v11/response.h#L558)

<a id="method-clear-body"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> clear_body

**METHOD** · *protocol::http::v11::response*

```cpp
response& clear_body();
```

Discards the body and restores empty-response framing.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:569](../../../../../include/protocol/http/v11/response.h#L569)

<a id="method-suppress-body"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> suppress_body

**METHOD** · *protocol::http::v11::response*

```cpp
response& suppress_body();
```

Discards body bytes while retaining the advertised framing for HEAD.

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:591](../../../../../include/protocol/http/v11/response.h#L591)

<a id="method-continue-100"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> continue_100

**METHOD** · *protocol::http::v11::response*

```cpp
response& continue_100();
```

Sets the HTTP response status to 100 (continue).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:601](../../../../../include/protocol/http/v11/response.h#L601)

<a id="method-switching-protocols-101"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> switching_protocols_101

**METHOD** · *protocol::http::v11::response*

```cpp
response& switching_protocols_101();
```

Sets the HTTP response status to 101 (switching protocols).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:605](../../../../../include/protocol/http/v11/response.h#L605)

<a id="method-ok-200"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ok_200

**METHOD** · *protocol::http::v11::response*

```cpp
response& ok_200();
```

Sets the HTTP response status to 200 (ok).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:609](../../../../../include/protocol/http/v11/response.h#L609)

<a id="method-created-201"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> created_201

**METHOD** · *protocol::http::v11::response*

```cpp
response& created_201();
```

Sets the HTTP response status to 201 (created).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:613](../../../../../include/protocol/http/v11/response.h#L613)

<a id="method-accepted-202"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> accepted_202

**METHOD** · *protocol::http::v11::response*

```cpp
response& accepted_202();
```

Sets the HTTP response status to 202 (accepted).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:617](../../../../../include/protocol/http/v11/response.h#L617)

<a id="method-non-authoritative-info-203"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> non_authoritative_info_203

**METHOD** · *protocol::http::v11::response*

```cpp
response& non_authoritative_info_203();
```

Sets the HTTP response status to 203 (non authoritative info).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:621](../../../../../include/protocol/http/v11/response.h#L621)

<a id="method-no-content-204"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> no_content_204

**METHOD** · *protocol::http::v11::response*

```cpp
response& no_content_204();
```

Sets the HTTP response status to 204 (no content).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:626](../../../../../include/protocol/http/v11/response.h#L626)

<a id="method-reset-content-205"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reset_content_205

**METHOD** · *protocol::http::v11::response*

```cpp
response& reset_content_205();
```

Sets the HTTP response status to 205 (reset content).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:630](../../../../../include/protocol/http/v11/response.h#L630)

<a id="method-partial-content-206"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> partial_content_206

**METHOD** · *protocol::http::v11::response*

```cpp
response& partial_content_206();
```

Sets the HTTP response status to 206 (partial content).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:634](../../../../../include/protocol/http/v11/response.h#L634)

<a id="method-multiple-choices-300"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> multiple_choices_300

**METHOD** · *protocol::http::v11::response*

```cpp
response& multiple_choices_300();
```

Sets the HTTP response status to 300 (multiple choices).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:638](../../../../../include/protocol/http/v11/response.h#L638)

<a id="method-moved-permanently-301"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> moved_permanently_301

**METHOD** · *protocol::http::v11::response*

```cpp
response& moved_permanently_301();
```

Sets the HTTP response status to 301 (moved permanently).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:642](../../../../../include/protocol/http/v11/response.h#L642)

<a id="method-found-302"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> found_302

**METHOD** · *protocol::http::v11::response*

```cpp
response& found_302();
```

Sets the HTTP response status to 302 (found).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:646](../../../../../include/protocol/http/v11/response.h#L646)

<a id="method-see-other-303"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> see_other_303

**METHOD** · *protocol::http::v11::response*

```cpp
response& see_other_303();
```

Sets the HTTP response status to 303 (see other).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:650](../../../../../include/protocol/http/v11/response.h#L650)

<a id="method-not-modified-304"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> not_modified_304

**METHOD** · *protocol::http::v11::response*

```cpp
response& not_modified_304();
```

Sets the HTTP response status to 304 (not modified).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:654](../../../../../include/protocol/http/v11/response.h#L654)

<a id="method-use-proxy-305"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> use_proxy_305

**METHOD** · *protocol::http::v11::response*

```cpp
response& use_proxy_305();
```

Sets the HTTP response status to 305 (use proxy).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:658](../../../../../include/protocol/http/v11/response.h#L658)

<a id="method-unused-306"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unused_306

**METHOD** · *protocol::http::v11::response*

```cpp
response& unused_306();
```

Sets the HTTP response status to 306 (unused).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:662](../../../../../include/protocol/http/v11/response.h#L662)

<a id="method-temporary-redirect-307"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> temporary_redirect_307

**METHOD** · *protocol::http::v11::response*

```cpp
response& temporary_redirect_307();
```

Sets the HTTP response status to 307 (temporary redirect).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:666](../../../../../include/protocol/http/v11/response.h#L666)

<a id="method-permanent-redirect-308"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> permanent_redirect_308

**METHOD** · *protocol::http::v11::response*

```cpp
response& permanent_redirect_308();
```

Sets the HTTP response status to 308 (permanent redirect).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:670](../../../../../include/protocol/http/v11/response.h#L670)

<a id="method-bad-request-400"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> bad_request_400

**METHOD** · *protocol::http::v11::response*

```cpp
response& bad_request_400();
```

Sets the HTTP response status to 400 (bad request).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:674](../../../../../include/protocol/http/v11/response.h#L674)

<a id="method-unauthorized-401"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unauthorized_401

**METHOD** · *protocol::http::v11::response*

```cpp
response& unauthorized_401();
```

Sets the HTTP response status to 401 (unauthorized).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:678](../../../../../include/protocol/http/v11/response.h#L678)

<a id="method-payment-required-402"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> payment_required_402

**METHOD** · *protocol::http::v11::response*

```cpp
response& payment_required_402();
```

Sets the HTTP response status to 402 (payment required).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:682](../../../../../include/protocol/http/v11/response.h#L682)

<a id="method-forbidden-403"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> forbidden_403

**METHOD** · *protocol::http::v11::response*

```cpp
response& forbidden_403();
```

Sets the HTTP response status to 403 (forbidden).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:686](../../../../../include/protocol/http/v11/response.h#L686)

<a id="method-not-found-404"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> not_found_404

**METHOD** · *protocol::http::v11::response*

```cpp
response& not_found_404();
```

Sets the HTTP response status to 404 (not found).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:690](../../../../../include/protocol/http/v11/response.h#L690)

<a id="method-method-not-allowed-405"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> method_not_allowed_405

**METHOD** · *protocol::http::v11::response*

```cpp
response& method_not_allowed_405();
```

Sets the HTTP response status to 405 (method not allowed).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:694](../../../../../include/protocol/http/v11/response.h#L694)

<a id="method-not-acceptable-406"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> not_acceptable_406

**METHOD** · *protocol::http::v11::response*

```cpp
response& not_acceptable_406();
```

Sets the HTTP response status to 406 (not acceptable).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:698](../../../../../include/protocol/http/v11/response.h#L698)

<a id="method-proxy-auth-required-407"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> proxy_auth_required_407

**METHOD** · *protocol::http::v11::response*

```cpp
response& proxy_auth_required_407();
```

Sets the HTTP response status to 407 (proxy auth required).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:702](../../../../../include/protocol/http/v11/response.h#L702)

<a id="method-request-timeout-408"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> request_timeout_408

**METHOD** · *protocol::http::v11::response*

```cpp
response& request_timeout_408();
```

Sets the HTTP response status to 408 (request timeout).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:707](../../../../../include/protocol/http/v11/response.h#L707)

<a id="method-conflict-409"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> conflict_409

**METHOD** · *protocol::http::v11::response*

```cpp
response& conflict_409();
```

Sets the HTTP response status to 409 (conflict).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:711](../../../../../include/protocol/http/v11/response.h#L711)

<a id="method-gone-410"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> gone_410

**METHOD** · *protocol::http::v11::response*

```cpp
response& gone_410();
```

Sets the HTTP response status to 410 (gone).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:715](../../../../../include/protocol/http/v11/response.h#L715)

<a id="method-length-required-411"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> length_required_411

**METHOD** · *protocol::http::v11::response*

```cpp
response& length_required_411();
```

Sets the HTTP response status to 411 (length required).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:719](../../../../../include/protocol/http/v11/response.h#L719)

<a id="method-precondition-failed-412"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> precondition_failed_412

**METHOD** · *protocol::http::v11::response*

```cpp
response& precondition_failed_412();
```

Sets the HTTP response status to 412 (precondition failed).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:723](../../../../../include/protocol/http/v11/response.h#L723)

<a id="method-content-too-large-413"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> content_too_large_413

**METHOD** · *protocol::http::v11::response*

```cpp
response& content_too_large_413();
```

Sets the HTTP response status to 413 (content too large).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:727](../../../../../include/protocol/http/v11/response.h#L727)

<a id="method-uri-too-long-414"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> uri_too_long_414

**METHOD** · *protocol::http::v11::response*

```cpp
response& uri_too_long_414();
```

Sets the HTTP response status to 414 (uri too long).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:731](../../../../../include/protocol/http/v11/response.h#L731)

<a id="method-unsupported-media-type-415"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unsupported_media_type_415

**METHOD** · *protocol::http::v11::response*

```cpp
response& unsupported_media_type_415();
```

Sets the HTTP response status to 415 (unsupported media type).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:735](../../../../../include/protocol/http/v11/response.h#L735)

<a id="method-range-not-satisfiable-416"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> range_not_satisfiable_416

**METHOD** · *protocol::http::v11::response*

```cpp
response& range_not_satisfiable_416();
```

Sets the HTTP response status to 416 (range not satisfiable).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:739](../../../../../include/protocol/http/v11/response.h#L739)

<a id="method-expectation-failed-417"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> expectation_failed_417

**METHOD** · *protocol::http::v11::response*

```cpp
response& expectation_failed_417();
```

Sets the HTTP response status to 417 (expectation failed).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:743](../../../../../include/protocol/http/v11/response.h#L743)

<a id="method-unused-418"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unused_418

**METHOD** · *protocol::http::v11::response*

```cpp
response& unused_418();
```

Sets the HTTP response status to 418 (unused).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:747](../../../../../include/protocol/http/v11/response.h#L747)

<a id="method-misdirected-request-421"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> misdirected_request_421

**METHOD** · *protocol::http::v11::response*

```cpp
response& misdirected_request_421();
```

Sets the HTTP response status to 421 (misdirected request).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:751](../../../../../include/protocol/http/v11/response.h#L751)

<a id="method-unprocessable-content-422"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unprocessable_content_422

**METHOD** · *protocol::http::v11::response*

```cpp
response& unprocessable_content_422();
```

Sets the HTTP response status to 422 (unprocessable content).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:755](../../../../../include/protocol/http/v11/response.h#L755)

<a id="method-upgrade-required-426"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> upgrade_required_426

**METHOD** · *protocol::http::v11::response*

```cpp
response& upgrade_required_426();
```

Sets the HTTP response status to 426 (upgrade required).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:759](../../../../../include/protocol/http/v11/response.h#L759)

<a id="method-request-header-fields-too-large-431"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> request_header_fields_too_large_431

**METHOD** · *protocol::http::v11::response*

```cpp
response& request_header_fields_too_large_431();
```

Sets the HTTP response status to 431 (request header fields too large).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:763](../../../../../include/protocol/http/v11/response.h#L763)

<a id="method-internal-server-error-500"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> internal_server_error_500

**METHOD** · *protocol::http::v11::response*

```cpp
response& internal_server_error_500();
```

Sets the HTTP response status to 500 (internal server error).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:768](../../../../../include/protocol/http/v11/response.h#L768)

<a id="method-not-implemented-501"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> not_implemented_501

**METHOD** · *protocol::http::v11::response*

```cpp
response& not_implemented_501();
```

Sets the HTTP response status to 501 (not implemented).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:772](../../../../../include/protocol/http/v11/response.h#L772)

<a id="method-bad-gateway-502"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> bad_gateway_502

**METHOD** · *protocol::http::v11::response*

```cpp
response& bad_gateway_502();
```

Sets the HTTP response status to 502 (bad gateway).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:776](../../../../../include/protocol/http/v11/response.h#L776)

<a id="method-service-unavailable-503"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> service_unavailable_503

**METHOD** · *protocol::http::v11::response*

```cpp
response& service_unavailable_503();
```

Sets the HTTP response status to 503 (service unavailable).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:780](../../../../../include/protocol/http/v11/response.h#L780)

<a id="method-gateway-timeout-504"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> gateway_timeout_504

**METHOD** · *protocol::http::v11::response*

```cpp
response& gateway_timeout_504();
```

Sets the HTTP response status to 504 (gateway timeout).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:784](../../../../../include/protocol/http/v11/response.h#L784)

<a id="method-http-version-not-supported-505"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> http_version_not_supported_505

**METHOD** · *protocol::http::v11::response*

```cpp
response& http_version_not_supported_505();
```

Sets the HTTP response status to 505 (http version not supported).

**Returns:** A reference to this object, for chaining calls.

**Source:** [include/protocol/http/v11/response.h:788](../../../../../include/protocol/http/v11/response.h#L788)
