<a id="connection"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/connection/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/connection/title.svg" alt="connection">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/connection/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/connection/title-dark.svg" alt="connection">
</a>

**Header:** [include/protocol/http/v11/connection.h](../../../../../include/protocol/http/v11/connection.h)

The hop-by-hop transport state that the semantic layer derives from a request. Framing and connection interpreters/rules read the parsed values of Connection, Transfer-Encoding, TE, Trailer, and Upgrade and mutate this state accordingly. It is deliberately separate from both the agnostic deserialization result and the request itself, because it

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[connection](#type-connection)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::connection

[persistent](#field-persistent) ·
[close_requested](#field-close-requested) ·
[transfer_codings](#field-transfer-codings) ·
[chunked](#field-chunked) ·
[te_codings](#field-te-codings) ·
[accepts_trailers](#field-accepts-trailers) ·
[trailer_names](#field-trailer-names) ·
[upgrade_offer](#field-upgrade-offer) ·
[options](#field-options) ·
[expects_continue](#field-expects-continue)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-connection"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> connection

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct connection { ... };
```

The hop-by-hop transport state that the semantic layer derives from a request. Framing and connection interpreters/rules read the parsed values of Connection, Transfer-Encoding, TE, Trailer, and Upgrade and mutate this state accordingly. It is deliberately separate from both the agnostic deserialization result and the request itself, because it

**Source:** [include/protocol/http/v11/connection.h:48](../../../../../include/protocol/http/v11/connection.h#L48)

<a id="field-persistent"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> persistent

**FIELD** · *protocol::http::v11::connection*

```cpp
bool persistent = true;
```

Whether the connection is to be kept alive after the current message. HTTP/1.1 defaults to persistent unless "Connection: close" is present.

**Value:** `true`

**Source:** [include/protocol/http/v11/connection.h:51](../../../../../include/protocol/http/v11/connection.h#L51)

<a id="field-close-requested"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> close_requested

**FIELD** · *protocol::http::v11::connection*

```cpp
bool close_requested = false;
```

Whether "Connection: close" was requested for this connection.

**Value:** `false`

**Source:** [include/protocol/http/v11/connection.h:53](../../../../../include/protocol/http/v11/connection.h#L53)

<a id="field-transfer-codings"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transfer_codings

**FIELD** · *protocol::http::v11::connection*

```cpp
std::vector<std::string_view> transfer_codings;
```

The transfer-coding names of Transfer-Encoding, in order (outermost last).

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/connection.h:55](../../../../../include/protocol/http/v11/connection.h#L55)

<a id="field-chunked"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> chunked

**FIELD** · *protocol::http::v11::connection*

```cpp
bool chunked = false;
```

Whether the final transfer-coding is "chunked" (message framing is then delimited by the chunked coding rather than by Content-Length).

**Value:** `false`

**Source:** [include/protocol/http/v11/connection.h:58](../../../../../include/protocol/http/v11/connection.h#L58)

<a id="field-te-codings"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> te_codings

**FIELD** · *protocol::http::v11::connection*

```cpp
std::vector<std::string_view> te_codings;
```

The transfer-coding names the client is willing to accept (TE), in order.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/connection.h:60](../../../../../include/protocol/http/v11/connection.h#L60)

<a id="field-accepts-trailers"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> accepts_trailers

**FIELD** · *protocol::http::v11::connection*

```cpp
bool accepts_trailers = false;
```

Whether the client signalled acceptance of the "trailers" TE token.

**Value:** `false`

**Source:** [include/protocol/http/v11/connection.h:62](../../../../../include/protocol/http/v11/connection.h#L62)

<a id="field-trailer-names"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> trailer_names

**FIELD** · *protocol::http::v11::connection*

```cpp
std::vector<std::string_view> trailer_names;
```

The header field names announced in the Trailer header, in order.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/connection.h:64](../../../../../include/protocol/http/v11/connection.h#L64)

<a id="field-upgrade-offer"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> upgrade_offer

**FIELD** · *protocol::http::v11::connection*

```cpp
std::vector<std::string_view> upgrade_offer;
```

The protocols offered in the Upgrade header, in order (empty when absent). Parsed and validated, but never acted upon: no upgrade is completed. See policies::allow_upgrade.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/connection.h:68](../../../../../include/protocol/http/v11/connection.h#L68)

<a id="field-options"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> options

**FIELD** · *protocol::http::v11::connection*

```cpp
std::vector<std::string_view> options;
```

Every connection-option token named by Connection, in order. These name header fields that are hop-by-hop for this connection.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/connection.h:71](../../../../../include/protocol/http/v11/connection.h#L71)

<a id="field-expects-continue"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> expects_continue

**FIELD** · *protocol::http::v11::connection*

```cpp
bool expects_continue = false;
```

Whether the client sent "Expect: 100-continue" and is therefore waiting for an interim response before sending the request body.

**Value:** `false`

**Source:** [include/protocol/http/v11/connection.h:74](../../../../../include/protocol/http/v11/connection.h#L74)
