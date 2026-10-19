<a id="helpers"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/helpers/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/helpers/index.svg" alt="helpers">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/helpers/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/helpers/index-dark.svg" alt="helpers">
</a>

**Header:** [include/protocol/http/common/helpers.h](../../../../../include/protocol/http/common/helpers.h)

Provides the http 1.1 helper functions.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::detail

[kF_token](#constant-kf-token) ·
[kF_vchar](#constant-kf-vchar) ·
[kF_obs_text](#constant-kf-obs-text) ·
[kF_ows](#constant-kf-ows) ·
[kF_pchar](#constant-kf-pchar) ·
[kF_qdtext](#constant-kf-qdtext) ·
[kF_ctext](#constant-kf-ctext) ·
[kF_etagc](#constant-kf-etagc) ·
[kF_cookie](#constant-kf-cookie) ·
[kF_cookie_av](#constant-kf-cookie-av) ·
[kF_atext](#constant-kf-atext) ·
[kF_dtext](#constant-kf-dtext) ·
[kF_hexdig](#constant-kf-hexdig) ·
[kF_unreserv](#constant-kf-unreserv) ·
[build_char_flags](#function-build-char-flags) ·
[kCharFlags](#constant-kcharflags)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[helpers](#type-helpers)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::helpers

[host_type](#enum-host-type) ·
[is_digit(c)](#method-is-digit) ·
[is_digit(c)](#method-is-digit-2) ·
[is_digits](#method-is-digits) ·
[parse_size_t](#method-parse-size-t) ·
[is_hex_digit(c)](#method-is-hex-digit) ·
[is_hex_digit(c)](#method-is-hex-digit-2) ·
[is_alpha(c)](#method-is-alpha) ·
[is_alpha(c)](#method-is-alpha-2) ·
[is_token(c)](#method-is-token) ·
[is_token(c)](#method-is-token-2) ·
[is_token(sv)](#method-is-token-3) ·
[is_unreserved(c)](#method-is-unreserved) ·
[is_unreserved(c)](#method-is-unreserved-2) ·
[is_sub_delim(c)](#method-is-sub-delim) ·
[is_sub_delim(c)](#method-is-sub-delim-2) ·
[is_pchar(c)](#method-is-pchar) ·
[is_pchar(c)](#method-is-pchar-2) ·
[is_vchar(c)](#method-is-vchar) ·
[is_vchar(c)](#method-is-vchar-2) ·
[is_qdtext(c)](#method-is-qdtext) ·
[is_qdtext(c)](#method-is-qdtext-2) ·
[is_obs_text(c)](#method-is-obs-text) ·
[is_obs_text(c)](#method-is-obs-text-2) ·
[is_ctext(c)](#method-is-ctext) ·
[is_ctext(c)](#method-is-ctext-2) ·
[is_atext(c)](#method-is-atext) ·
[is_atext(c)](#method-is-atext-2) ·
[is_dtext(c)](#method-is-dtext) ·
[is_dtext(c)](#method-is-dtext-2) ·
[is_etagc(c)](#method-is-etagc) ·
[is_etagc(c)](#method-is-etagc-2) ·
[is_cookie_octet(c)](#method-is-cookie-octet) ·
[is_cookie_octet(c)](#method-is-cookie-octet-2) ·
[is_cookie_av_octet(c)](#method-is-cookie-av-octet) ·
[is_cookie_av_octet(c)](#method-is-cookie-av-octet-2) ·
[is_entity_tag](#method-is-entity-tag) ·
[is_ows(c)](#method-is-ows) ·
[is_ows(c)](#method-is-ows-2) ·
[is_qvalue](#method-is-qvalue) ·
[consume_weight](#method-consume-weight) ·
[tolower_ascii(c)](#method-tolower-ascii) ·
[tolower_ascii(c)](#method-tolower-ascii-2) ·
[ows_ltrim](#method-ows-ltrim) ·
[ows_rtrim](#method-ows-rtrim) ·
[ows_trim](#method-ows-trim) ·
[skip_n_hextets](#method-skip-n-hextets) ·
[is_port](#method-is-port) ·
[is_dec_octet(sv, digits, off)](#method-is-dec-octet) ·
[is_dec_octet(sv)](#method-is-dec-octet-2) ·
[is_ip_v_future](#method-is-ip-v-future) ·
[is_ip_v6_address](#method-is-ip-v6-address) ·
[is_ip_v4_address](#method-is-ip-v4-address) ·
[is_ip_literal](#method-is-ip-literal) ·
[is_reg_name](#method-is-reg-name) ·
[is_token68](#method-is-token68) ·
[is_base64_char](#method-is-base64-char) ·
[check_base64_value](#method-check-base64-value) ·
[consume_token](#method-consume-token) ·
[consume_quoted_string](#method-consume-quoted-string) ·
[consume_comment](#method-consume-comment) ·
[consume_cfws](#method-consume-cfws) ·
[consume_dot_atom](#method-consume-dot-atom) ·
[consume_domain_literal](#method-consume-domain-literal) ·
[consume_addr_spec](#method-consume-addr-spec) ·
[consume_word](#method-consume-word) ·
[check_mailbox](#method-check-mailbox) ·
[consume_token_or_quoted_string](#method-consume-token-or-quoted-string) ·
[is_cookie_value](#method-is-cookie-value) ·
[is_cookie_pair](#method-is-cookie-pair) ·
[is_cookie_av](#method-is-cookie-av) ·
[is_directive](#method-is-directive) ·
[consume_parameter](#method-consume-parameter) ·
[consume_extension_parameter](#method-consume-extension-parameter) ·
[for_each_parameter](#method-for-each-parameter) ·
[for_each_forwarded_pair](#method-for-each-forwarded-pair) ·
[for_each_list_element](#method-for-each-list-element) ·
[check_auth_param_list](#method-check-auth-param-list) ·
[check_credentials](#method-check-credentials) ·
[challenge_opens_param_list](#method-challenge-opens-param-list) ·
[check_challenge_list](#method-check-challenge-list) ·
[consume_product](#method-consume-product) ·
[check_product_list](#method-check-product-list) ·
[check_uri_host](#method-check-uri-host) ·
[is_uri_scheme](#method-is-uri-scheme) ·
[consume_uri_scheme](#method-consume-uri-scheme) ·
[consume_uri_pct_encoded](#method-consume-uri-pct-encoded) ·
[percent_decode_validate](#method-percent-decode-validate) ·
[percent_decode_in_place](#method-percent-decode-in-place) ·
[check_uri_userinfo](#method-check-uri-userinfo) ·
[check_host_port(sv, uri_host, port, type)](#method-check-host-port) ·
[check_host_port(sv)](#method-check-host-port-2) ·
[check_serialized_origin](#method-check-serialized-origin) ·
[check_uri_authority](#method-check-uri-authority) ·
[check_uri_reference](#method-check-uri-reference) ·
[split_authority_host_port](#method-split-authority-host-port) ·
[try_to_deserialize_as_authority_form](#method-try-to-deserialize-as-authority-form) ·
[try_to_deserialize_as_asterisk_form](#method-try-to-deserialize-as-asterisk-form) ·
[try_to_deserialize_as_origin_form](#method-try-to-deserialize-as-origin-form) ·
[default_port_for_scheme](#method-default-port-for-scheme) ·
[ports_equivalent](#method-ports-equivalent) ·
[try_to_deserialize_as_absolute_form](#method-try-to-deserialize-as-absolute-form) ·
[try_to_deserialize_as_absolute_path](#method-try-to-deserialize-as-absolute-path) ·
[try_to_deserialize_as_query](#method-try-to-deserialize-as-query) ·
[iequals](#method-iequals) ·
[split_query_parameters](#method-split-query-parameters)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="constant-kf-token"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_token

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_token = 0x0001;
```

Provides the http 1.1 helper functions. Each bit encodes one RFC character-class predicate for a byte value. kCharFlags[c] is a bitmask; is_X(c) reduces to a single table lookup and a bitmask test (2 instructions) instead of a chain of comparisons. The table is 256 * 2 = 512 bytes; fits in one L1 cache line group and is shared across

**Value:** `0x0001`

**Source:** [include/protocol/http/common/helpers.h:54](../../../../../include/protocol/http/common/helpers.h#L54)

<a id="constant-kf-vchar"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_vchar

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_vchar = 0x0002;
```

Defines the f vchar constant.

**Value:** `0x0002`

**Source:** [include/protocol/http/common/helpers.h:55](../../../../../include/protocol/http/common/helpers.h#L55)

<a id="constant-kf-obs-text"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_obs_text

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_obs_text = 0x0004;
```

Defines the f obs text constant.

**Value:** `0x0004`

**Source:** [include/protocol/http/common/helpers.h:56](../../../../../include/protocol/http/common/helpers.h#L56)

<a id="constant-kf-ows"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_ows

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_ows = 0x0008;
```

Defines the f ows constant.

**Value:** `0x0008`

**Source:** [include/protocol/http/common/helpers.h:57](../../../../../include/protocol/http/common/helpers.h#L57)

<a id="constant-kf-pchar"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_pchar

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_pchar = 0x0010;
```

Defines the f pchar constant.

**Value:** `0x0010`

**Source:** [include/protocol/http/common/helpers.h:58](../../../../../include/protocol/http/common/helpers.h#L58)

<a id="constant-kf-qdtext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_qdtext

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_qdtext = 0x0020;
```

Defines the f qdtext constant.

**Value:** `0x0020`

**Source:** [include/protocol/http/common/helpers.h:59](../../../../../include/protocol/http/common/helpers.h#L59)

<a id="constant-kf-ctext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_ctext

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_ctext = 0x0040;
```

Defines the f ctext constant.

**Value:** `0x0040`

**Source:** [include/protocol/http/common/helpers.h:60](../../../../../include/protocol/http/common/helpers.h#L60)

<a id="constant-kf-etagc"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_etagc

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_etagc = 0x0080;
```

Defines the f etagc constant.

**Value:** `0x0080`

**Source:** [include/protocol/http/common/helpers.h:61](../../../../../include/protocol/http/common/helpers.h#L61)

<a id="constant-kf-cookie"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_cookie

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_cookie = 0x0100;
```

Defines the f cookie constant.

**Value:** `0x0100`

**Source:** [include/protocol/http/common/helpers.h:62](../../../../../include/protocol/http/common/helpers.h#L62)

<a id="constant-kf-cookie-av"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_cookie_av

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_cookie_av = 0x0200;
```

Defines the f cookie av constant.

**Value:** `0x0200`

**Source:** [include/protocol/http/common/helpers.h:63](../../../../../include/protocol/http/common/helpers.h#L63)

<a id="constant-kf-atext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_atext

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_atext = 0x0400;
```

Defines the f atext constant.

**Value:** `0x0400`

**Source:** [include/protocol/http/common/helpers.h:64](../../../../../include/protocol/http/common/helpers.h#L64)

<a id="constant-kf-dtext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_dtext

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_dtext = 0x0800;
```

Defines the f dtext constant.

**Value:** `0x0800`

**Source:** [include/protocol/http/common/helpers.h:65](../../../../../include/protocol/http/common/helpers.h#L65)

<a id="constant-kf-hexdig"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_hexdig

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_hexdig = 0x1000;
```

Defines the f hexdig constant.

**Value:** `0x1000`

**Source:** [include/protocol/http/common/helpers.h:66](../../../../../include/protocol/http/common/helpers.h#L66)

<a id="constant-kf-unreserv"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kF_unreserv

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr std::uint16_t kF_unreserv = 0x2000;
```

Defines the f unreserv constant.

**Value:** `0x2000`

**Source:** [include/protocol/http/common/helpers.h:67](../../../../../include/protocol/http/common/helpers.h#L67)

<a id="function-build-char-flags"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> build_char_flags

**FUNCTION** · *protocol::http::detail*

```cpp
static constexpr std::array<std::uint16_t, 256> build_char_flags() noexcept;
```

The table is built at compile time, so the compiler can optimize it into a static data segment.

**Returns:** A value of type `std::array<std::uint16_t, 256>`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:75](../../../../../include/protocol/http/common/helpers.h#L75)

<a id="constant-kcharflags"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kCharFlags

**CONSTANT** · *protocol::http::detail*

```cpp
static constexpr auto kCharFlags = build_char_flags();
```

Defines the char flags constant.

**Value:** `build_char_flags()`

**Source:** [include/protocol/http/common/helpers.h:140](../../../../../include/protocol/http/common/helpers.h#L140)

<a id="type-helpers"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> helpers

**TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
struct helpers { ... };
```

Provides the http 1.1 helper functions.

**Source:** [include/protocol/http/common/helpers.h:150](../../../../../include/protocol/http/common/helpers.h#L150)

<a id="enum-host-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> host_type

**ENUM** · *protocol::http::helpers*

```cpp
enum class host_type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
```

Enumerates host type values.

**Values:**

- **kUnknown:** Unknown. Declaration: `kUnknown`.
- **kIpLiteral:** Ip literal. Declaration: `kIpLiteral`.
- **kIpV4Address:** Ip v4 address. Declaration: `kIpV4Address`.
- **kRegName:** Reg name. Declaration: `kRegName`.

**Source:** [include/protocol/http/common/helpers.h:154](../../../../../include/protocol/http/common/helpers.h#L154)

<a id="method-is-digit"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_digit

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_digit(unsigned char c) noexcept;
```

DIGIT = %x30-39    ; "0" - "9"

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:162](../../../../../include/protocol/http/common/helpers.h#L162)

<a id="method-is-digit-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_digit

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_digit(char c) noexcept;
```

Reports whether digit applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:165](../../../../../include/protocol/http/common/helpers.h#L165)

<a id="method-is-digits"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_digits

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_digits(std::string_view sv) noexcept;
```

1*DIGIT Validates that sv, as a whole, is a non-empty run of decimal digits. This is the syntactic shape shared by delta-seconds (RFC 9111 S1.2.2) and Content-Length (RFC 9110 S8.6). Any overflow handling is semantic processing and does not affect this purely syntactic predicate.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:180](../../../../../include/protocol/http/common/helpers.h#L180)

<a id="method-parse-size-t"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parse_size_t

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool parse_size_t(std::string_view sv,
                                     std::size_t& out) noexcept;
```

Validates that sv is a non-empty run of decimal digits (exactly like is_digits) and, on success, converts it to a std::size_t in out. This is the producing counterpart of the purely syntactic is_digits and is shared by the Content-Length (RFC 9110 S8.6) and Max-Forwards (RFC 9110 S7.6.2) producer overloads. Returns false on a non-1*DIGIT value or

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `std::size_t& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:201](../../../../../include/protocol/http/common/helpers.h#L201)

<a id="method-is-hex-digit"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_hex_digit

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_hex_digit(unsigned char c) noexcept;
```

HEXDIG = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:227](../../../../../include/protocol/http/common/helpers.h#L227)

<a id="method-is-hex-digit-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_hex_digit

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_hex_digit(char c) noexcept;
```

Reports whether hex digit applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:230](../../../../../include/protocol/http/common/helpers.h#L230)

<a id="method-is-alpha"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_alpha

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_alpha(unsigned char c) noexcept;
```

ALPHA = %x41-5A / %x61-7A ; A-Z / a-z

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:241](../../../../../include/protocol/http/common/helpers.h#L241)

<a id="method-is-alpha-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_alpha

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_alpha(char c) noexcept;
```

Reports whether alpha applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:244](../../../../../include/protocol/http/common/helpers.h#L244)

<a id="method-is-token"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_token(unsigned char c) noexcept;
```

token  = 1*tchar

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:260](../../../../../include/protocol/http/common/helpers.h#L260)

<a id="method-is-token-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_token(char c) noexcept;
```

Reports whether token applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:263](../../../../../include/protocol/http/common/helpers.h#L263)

<a id="method-is-token-3"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_token(std::string_view sv) noexcept;
```

token = 1*tchar Validates that sv, as a whole, is exactly one token: non-empty, with every character satisfying tchar. Unlike consume_token(), which greedily consumes only the tchar prefix of sv, this rejects any trailing bytes that are not part of the token.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:278](../../../../../include/protocol/http/common/helpers.h#L278)

<a id="method-is-unreserved"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_unreserved

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_unreserved(unsigned char c) noexcept;
```

unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~" ALPHA      = %x41-5A / %x61-7A   ; A-Z / a-z DIGIT      = %x30-39             ; 0-9

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:295](../../../../../include/protocol/http/common/helpers.h#L295)

<a id="method-is-unreserved-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_unreserved

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_unreserved(char c) noexcept;
```

Reports whether unreserved applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:298](../../../../../include/protocol/http/common/helpers.h#L298)

<a id="method-is-sub-delim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_sub_delim

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_sub_delim(unsigned char c) noexcept;
```

sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:309](../../../../../include/protocol/http/common/helpers.h#L309)

<a id="method-is-sub-delim-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_sub_delim

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_sub_delim(char c) noexcept;
```

Reports whether sub delim applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:313](../../../../../include/protocol/http/common/helpers.h#L313)

<a id="method-is-pchar"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_pchar

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_pchar(unsigned char c) noexcept;
```

pchar = unreserved / pct-encoded / sub-delims / ":" / "@" unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~" pct-encoded = "%" HEXDIG HEXDIG sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "=" ALPHA  = %x41-5A / %x61-7A   ; A-Z / a-z DIGIT  = %x30-39             ; 0-9 HEXDIG = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:332](../../../../../include/protocol/http/common/helpers.h#L332)

<a id="method-is-pchar-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_pchar

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_pchar(char c) noexcept;
```

Reports whether pchar applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:335](../../../../../include/protocol/http/common/helpers.h#L335)

<a id="method-is-vchar"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_vchar

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_vchar(unsigned char c) noexcept;
```

VCHAR = %x21-7E ; visible (printing) characters

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:346](../../../../../include/protocol/http/common/helpers.h#L346)

<a id="method-is-vchar-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_vchar

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_vchar(char c) noexcept;
```

Reports whether vchar applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:349](../../../../../include/protocol/http/common/helpers.h#L349)

<a id="method-is-qdtext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_qdtext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_qdtext(unsigned char c) noexcept;
```

qdtext = HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text HTAB     = %x09 SP       = %x20 obs-text = %x80-FF Note: DQUOTE (%x22) and "\" (%x5C) are excluded and only allowed via quoted-pair.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:366](../../../../../include/protocol/http/common/helpers.h#L366)

<a id="method-is-qdtext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_qdtext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_qdtext(char c) noexcept;
```

Reports whether qdtext applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:369](../../../../../include/protocol/http/common/helpers.h#L369)

<a id="method-is-obs-text"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_obs_text

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_obs_text(unsigned char c) noexcept;
```

obs-text = %x80-FF Note: obs-text represents obsolete text allowed for backward compatibility with legacy implementations.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:382](../../../../../include/protocol/http/common/helpers.h#L382)

<a id="method-is-obs-text-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_obs_text

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_obs_text(char c) noexcept;
```

Reports whether obs text applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:385](../../../../../include/protocol/http/common/helpers.h#L385)

<a id="method-is-ctext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ctext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ctext(unsigned char c) noexcept;
```

ctext = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text HTAB     = %x09 SP       = %x20 obs-text = %x80-FF Note: "(" (%x28), ")" (%x29), and "\" (%x5C) are excluded; inside a comment they are only allowed structurally (nested comment) or via quoted-pair.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:403](../../../../../include/protocol/http/common/helpers.h#L403)

<a id="method-is-ctext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ctext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ctext(char c) noexcept;
```

Reports whether ctext applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:406](../../../../../include/protocol/http/common/helpers.h#L406)

<a id="method-is-atext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_atext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_atext(unsigned char c) noexcept;
```

atext = ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "/" / "=" / "?" / "^" / "_" / "`" / "{" / "|" / "}" / "~" The atom character set used by dot-atom-text and atom (RFC 5322 S3.2.3), imported into HTTP via the From "mailbox" grammar (RFC 9110 S10.1.2).

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:420](../../../../../include/protocol/http/common/helpers.h#L420)

<a id="method-is-atext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_atext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_atext(char c) noexcept;
```

Reports whether atext applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:423](../../../../../include/protocol/http/common/helpers.h#L423)

<a id="method-is-dtext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dtext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_dtext(unsigned char c) noexcept;
```

dtext = %d33-90 / %d94-126 / obs-dtext The unquoted character set allowed inside a domain-literal. "[" (%d91), "\" (%d92), and "]" (%d93) are excluded; inside a domain-literal they are only allowed structurally (the delimiters) or via quoted-pair. obs-text (%x80-FF) covers the obs-dtext octet allowance in the HTTP parsing context.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:439](../../../../../include/protocol/http/common/helpers.h#L439)

<a id="method-is-dtext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dtext

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_dtext(char c) noexcept;
```

Reports whether dtext applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:442](../../../../../include/protocol/http/common/helpers.h#L442)

<a id="method-is-etagc"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_etagc

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_etagc(unsigned char c) noexcept;
```

etagc = %x21 / %x23-7E / obs-text ; VCHAR except DQUOTE, plus obs-text VCHAR    = %x21-7E DQUOTE   = %x22 obs-text = %x80-FF

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:457](../../../../../include/protocol/http/common/helpers.h#L457)

<a id="method-is-etagc-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_etagc

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_etagc(char c) noexcept;
```

Reports whether etagc applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:460](../../../../../include/protocol/http/common/helpers.h#L460)

<a id="method-is-cookie-octet"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_octet

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_cookie_octet(unsigned char c) noexcept;
```

cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E ; US-ASCII characters excluding CTLs, whitespace, DQUOTE, ; comma, semicolon, and backslash

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:472](../../../../../include/protocol/http/common/helpers.h#L472)

<a id="method-is-cookie-octet-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_octet

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_cookie_octet(char c) noexcept;
```

Reports whether cookie octet applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:475](../../../../../include/protocol/http/common/helpers.h#L475)

<a id="method-is-cookie-av-octet"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_av_octet

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_cookie_av_octet(unsigned char c) noexcept;
```

Both path-value and extension-av are defined as any CHAR except CTLs or a ";". CHAR is %x01-7F and CTLs are %x00-1F / %x7F, so the octet class reduces to %x20-7E excluding ";" (%x3B). This is deliberately broad: it admits SP, comma, "=", and DQUOTE, which is why an Expires date such as "Wed, 09 Jun 2021 10:18:14 GMT" is a syntactically valid

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:489](../../../../../include/protocol/http/common/helpers.h#L489)

<a id="method-is-cookie-av-octet-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_av_octet

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_cookie_av_octet(char c) noexcept;
```

Reports whether cookie av octet applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:492](../../../../../include/protocol/http/common/helpers.h#L492)

<a id="method-is-entity-tag"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_entity_tag

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_entity_tag(std::string_view sv) noexcept;
```

entity-tag = [ weak ] opaque-tag weak       = %s"W/" opaque-tag = DQUOTE *etagc DQUOTE Validates that sv is exactly one entity-tag (optionally weak), consuming the whole of sv. The "W/" prefix is case-sensitive, as %s denotes a case-sensitive string in the ABNF.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:508](../../../../../include/protocol/http/common/helpers.h#L508)

<a id="method-is-ows"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ows

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ows(unsigned char c) noexcept;
```

OWS = *( SP / HTAB ) SP   = %x20 HTAB = %x09 Note: OWS is allowed only where explicitly defined by the ABNF.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:537](../../../../../include/protocol/http/common/helpers.h#L537)

<a id="method-is-ows-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ows

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ows(char c) noexcept;
```

Reports whether ows applies.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:540](../../../../../include/protocol/http/common/helpers.h#L540)

<a id="method-is-qvalue"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_qvalue

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_qvalue(std::string_view sv) noexcept;
```

qvalue             | ( "0" [ "." 0*3DIGIT ] ) / | ( "1" [ "." 0*3("0") ] )

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:549](../../../../../include/protocol/http/common/helpers.h#L549)

<a id="method-consume-weight"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_weight

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_weight(std::string_view sv) noexcept;
```

weight = OWS ";" OWS "q=" qvalue Validates that sv is exactly one weight parameter (the case-insensitive "q" name, no whitespace around "="), consuming the whole of sv.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:569](../../../../../include/protocol/http/common/helpers.h#L569)

<a id="method-tolower-ascii"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tolower_ascii

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr unsigned char tolower_ascii(unsigned char c) noexcept;
```

Converts character to its lower-case version.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `unsigned char c`.

**Returns:** A value of type `unsigned char`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:595](../../../../../include/protocol/http/common/helpers.h#L595)

<a id="method-tolower-ascii-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tolower_ascii

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr char tolower_ascii(char c) noexcept;
```

Converts an ASCII character to lowercase.

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A value of type `char`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:598](../../../../../include/protocol/http/common/helpers.h#L598)

<a id="method-ows-ltrim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ows_ltrim

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr void ows_ltrim(std::string_view& sv) noexcept;
```

Performs OWS trimming on the left side of the provided string_view.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view& sv`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:606](../../../../../include/protocol/http/common/helpers.h#L606)

<a id="method-ows-rtrim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ows_rtrim

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr void ows_rtrim(std::string_view& sv) noexcept;
```

Performs OWS trimming on the right side of the provided string_view.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view& sv`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:616](../../../../../include/protocol/http/common/helpers.h#L616)

<a id="method-ows-trim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ows_trim

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr void ows_trim(std::string_view& sv) noexcept;
```

Performs OWS trimming on left & right sides of the provided string_view.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view& sv`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:626](../../../../../include/protocol/http/common/helpers.h#L626)

<a id="method-skip-n-hextets"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> skip_n_hextets

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr int skip_n_hextets(std::string_view sv);
```

h16            | 1*4HEXDIG

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A value of type `int`.

**Source:** [include/protocol/http/common/helpers.h:636](../../../../../include/protocol/http/common/helpers.h#L636)

<a id="method-is-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_port

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_port(std::string_view sv);
```

port           | *DIGIT

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:664](../../../../../include/protocol/http/common/helpers.h#L664)

<a id="method-is-dec-octet"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dec_octet

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_dec_octet(std::string_view sv, std::size_t digits,
                                     std::size_t off);
```

dec-octet      | DIGIT                    ; 0-9 | / %x31-39 DIGIT          ; 10-99 | / "1" 2DIGIT             ; 100-199 | / "2" %x30-34 DIGIT      ; 200-249 | / "25" %x30-35           ; 250-255

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **digits:** Number of digits to process. Declaration: `std::size_t digits`.
- **off:** Offset into the input or output sequence. Declaration: `std::size_t off`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:682](../../../../../include/protocol/http/common/helpers.h#L682)

<a id="method-is-dec-octet-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dec_octet

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_dec_octet(std::string_view sv) noexcept;
```

dec-octet = DIGIT                 ; 0-9 / %x31-39 DIGIT         ; 10-99 / "1" 2DIGIT            ; 100-199 / "2" %x30-34 DIGIT     ; 200-249 / "25" %x30-35          ; 250-255 Validates that sv, as a whole, is exactly one canonical dec-octet: a decimal integer in the range 0..255 with no leading zeroes (except the single value "0"). This is the RFC

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:719](../../../../../include/protocol/http/common/helpers.h#L719)

<a id="method-is-ip-v-future"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_v_future

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ip_v_future(std::string_view sv);
```

IPvFuture    | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:745](../../../../../include/protocol/http/common/helpers.h#L745)

<a id="method-is-ip-v6-address"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_v6_address

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ip_v6_address(std::string_view sv);
```

IPv6address    | 6( h16 ":" ) ls32 | / "::" 5( h16 ":" ) ls32 | / [ h16 ] "::" 4( h16 ":" ) ls32 | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32 | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32 | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32 | / [ *4( h16 ":" ) h16 ] "::" ls32 | / [ *5( h16 ":" ) h16 ] "::" h16 | / [ *6( h16 ":" ) h16 ] "::" ls32

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:786](../../../../../include/protocol/http/common/helpers.h#L786)

<a id="method-is-ip-v4-address"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_v4_address

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ip_v4_address(std::string_view sv);
```

IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:852](../../../../../include/protocol/http/common/helpers.h#L852)

<a id="method-is-ip-literal"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_literal

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_ip_literal(std::string_view sv);
```

IP-literal     | "[" ( IPv6address / IPvFuture ) "]"

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:880](../../../../../include/protocol/http/common/helpers.h#L880)

<a id="method-is-reg-name"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_reg_name

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_reg_name(std::string_view sv);
```

reg-name       | *( unreserved / pct-encoded / sub-delims )

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:897](../../../../../include/protocol/http/common/helpers.h#L897)

<a id="method-is-token68"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token68

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_token68(std::string_view sv) noexcept;
```

token68 = 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" / "/" ) *"=" Validates that sv, as a whole, is exactly one token68: a non-empty run of unreserved characters plus "+"/"/", followed by any number (possibly zero) of "=" padding characters. At least one leading (non-"=") character is required, and no other bytes may follow the "=" run.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:925](../../../../../include/protocol/http/common/helpers.h#L925)

<a id="method-is-base64-char"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_base64_char

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_base64_char(char c) noexcept;
```

base64-character = ALPHA / DIGIT / "+" / "/"

**Parameters:**

- **c:** Character to inspect or write. Declaration: `char c`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:946](../../../../../include/protocol/http/common/helpers.h#L946)

<a id="method-check-base64-value"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_base64_value

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_base64_value(std::string_view sv) noexcept;
```

base64-value-non-empty = ( 1*base64-data [ base64-padding ] ) / base64-padding base64-data            = 4base64-character base64-padding         = ( 2base64-character "==" ) / ( 3base64-character "=" ) base64-character       = ALPHA / DIGIT / "+" / "/" Validates that sv, as a whole, is a non-empty canonical base64 value: a positive multiple of

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:970](../../../../../include/protocol/http/common/helpers.h#L970)

<a id="method-consume-token"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_token

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr std::string_view consume_token(
      std::string_view sv) noexcept;
```

method  | token token   | 1*tchar tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." / | "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1001](../../../../../include/protocol/http/common/helpers.h#L1001)

<a id="method-consume-quoted-string"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_quoted_string

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr std::string_view consume_quoted_string(
      std::string_view sv) noexcept;
```

quoted-string = DQUOTE *( qdtext / quoted-pair ) DQUOTE quoted-pair   = "\" ( HTAB / SP / VCHAR / obs-text ) Consumes a quoted-string from the provided string_view, validating that every unescaped character is qdtext and every escaped character is a valid quoted-pair octet.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1024](../../../../../include/protocol/http/common/helpers.h#L1024)

<a id="method-consume-comment"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_comment

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr std::string_view consume_comment(
      std::string_view sv) noexcept;
```

comment     = "(" *( ctext / quoted-pair / comment ) ")" quoted-pair = "\" ( HTAB / SP / VCHAR / obs-text ) Consumes a single comment from the front of sv and returns the consumed prefix (including the surrounding parentheses), or an empty string_view if sv does not begin with a well-formed comment. Comments nest, so a depth counter tracks

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1067](../../../../../include/protocol/http/common/helpers.h#L1067)

<a id="method-consume-cfws"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_cfws

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_cfws(std::string_view sv,
                                     std::size_t& i) noexcept;
```

CFWS = (1*([FWS] comment) [FWS]) / FWS FWS  = ([*WSP CRLF] 1*WSP) / obs-FWS WSP  = SP / HTAB Consumes an optional run of folding whitespace and comments starting at sv[i], advancing i past it. Because HTTP field values are already normalized (no CR/LF, no surrounding OWS), FWS collapses to runs of WSP (SP / HTAB, i.e. is_ows), so CFWS reduces to

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **i:** Zero-based index of the requested element. Declaration: `std::size_t& i`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1115](../../../../../include/protocol/http/common/helpers.h#L1115)

<a id="method-consume-dot-atom"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_dot_atom

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_dot_atom(std::string_view sv,
                                         std::size_t& i) noexcept;
```

dot-atom      = [CFWS] dot-atom-text [CFWS] dot-atom-text = 1*atext *( "." 1*atext ) Consumes a single dot-atom starting at sv[i], advancing i past it, and returns true on success. Optional leading CFWS is consumed first (via consume_cfws), then dot-atom-text: one or more atext (is_atext) runs separated by single "." characters. A leading,

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **i:** Zero-based index of the requested element. Declaration: `std::size_t& i`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1147](../../../../../include/protocol/http/common/helpers.h#L1147)

<a id="method-consume-domain-literal"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_domain_literal

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_domain_literal(std::string_view sv,
                                               std::size_t& i) noexcept;
```

domain-literal = [CFWS] "[" *([FWS] dtext) [FWS] "]" [CFWS] Consumes a single domain-literal starting at sv[i], advancing i past it and returning true on success. Optional leading CFWS is consumed first, then a mandatory "[", then any run of dtext (is_dtext) interleaved with FWS which, in the normalized HTTP field-value context, collapses to WSP

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **i:** Zero-based index of the requested element. Declaration: `std::size_t& i`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1176](../../../../../include/protocol/http/common/helpers.h#L1176)

<a id="method-consume-addr-spec"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_addr_spec

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_addr_spec(std::string_view sv,
                                          std::size_t& i) noexcept;
```

addr-spec  = local-part "@" domain local-part = dot-atom / quoted-string / obs-local-part domain     = dot-atom / domain-literal / obs-domain Consumes a single addr-spec starting at sv[i], advancing i past it and returning true on success. The local-part is either a quoted-string (when the first non-CFWS octet is DQUOTE, consumed via

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **i:** Zero-based index of the requested element. Declaration: `std::size_t& i`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1204](../../../../../include/protocol/http/common/helpers.h#L1204)

<a id="method-consume-word"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_word

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_word(std::string_view sv,
                                     std::size_t& i) noexcept;
```

word          = atom / quoted-string atom          = [CFWS] 1*atext [CFWS] quoted-string = [CFWS] DQUOTE *([FWS] qcontent) [FWS] DQUOTE [CFWS] Consumes a single word starting at sv[i], advancing i past it and returning true on success. A word is either a quoted-string (when the first non-CFWS octet is DQUOTE, consumed via consume_quoted_string

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **i:** Zero-based index of the requested element. Declaration: `std::size_t& i`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1245](../../../../../include/protocol/http/common/helpers.h#L1245)

<a id="method-check-mailbox"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_mailbox

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_mailbox(std::string_view sv) noexcept;
```

mailbox      = name-addr / addr-spec name-addr    = [display-name] angle-addr angle-addr   = [CFWS] "<" addr-spec ">" [CFWS] display-name = phrase phrase       = 1*word Validates that sv, as a whole, is exactly one mailbox. This is the single-mailbox value form imported by the From header (RFC 9110 S10.1.2); From is not a list field, so no

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1279](../../../../../include/protocol/http/common/helpers.h#L1279)

<a id="method-consume-token-or-quoted-string"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_token_or_quoted_string

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr std::string_view consume_token_or_quoted_string(
      std::string_view sv) noexcept;
```

value = token / quoted-string Consumes a quoted-string (via consume_quoted_string) when sv starts with DQUOTE, or a token (via consume_token) otherwise. Returns an empty string_view when neither production matches (e.g., an empty token or an unterminated/invalid quoted-string).

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1319](../../../../../include/protocol/http/common/helpers.h#L1319)

<a id="method-is-cookie-value"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_value

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_cookie_value(std::string_view sv) noexcept;
```

cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE ) Validates that sv, as a whole, is exactly one cookie-value, the shared value form of the Cookie (RFC 6265 S4.2.1) and Set-Cookie (RFC 6265 S4.1.1) cookie-pair. The value is either a (possibly empty) run of cookie-octets, or the same run wrapped in a pair of DQUOTE delimiters. The

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1339](../../../../../include/protocol/http/common/helpers.h#L1339)

<a id="method-is-cookie-pair"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_pair

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_cookie_pair(std::string_view sv) noexcept;
```

cookie-pair  = cookie-name "=" cookie-value cookie-name  = token cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE ) Validates that sv, as a whole, is exactly one cookie-pair, the shared pair form of the Cookie (RFC 6265 S4.2.1) and Set-Cookie (RFC 6265 S4.1.1) fields. A non-empty token cookie-name and a mandatory "=" (with no

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1370](../../../../../include/protocol/http/common/helpers.h#L1370)

<a id="method-is-cookie-av"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_av

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_cookie_av(std::string_view sv) noexcept;
```

cookie-av    = expires-av / max-age-av / domain-av / path-av / secure-av / httponly-av / extension-av extension-av = <any CHAR except CTLs or ";"> Validates that sv, as a whole, is exactly one cookie-av of a Set-Cookie field (RFC 6265 S4.1.1). Every named alternative (Expires, Max-Age, Domain, Path, Secure, HttpOnly) is subsumed by extension-av,

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1398](../../../../../include/protocol/http/common/helpers.h#L1398)

<a id="method-is-directive"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_directive

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_directive(std::string_view sv) noexcept;
```

directive = token [ "=" ( token / quoted-string ) ] Validates that sv, as a whole, is exactly one parameterized directive: a non-empty token optionally followed by "=" and a token or quoted- string argument, with no surrounding whitespace around "=". This is the shared shape of cache-directive (RFC 9111 S5.2) and extension-pragma (RFC 9111 S5.4);

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1422](../../../../../include/protocol/http/common/helpers.h#L1422)

<a id="method-consume-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_parameter

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_parameter(std::string_view sv,
                                          std::size_t& bytes_used,
                                          bool allow_bws) noexcept;
```

parameter       = parameter-name "=" parameter-value parameter-name  = token parameter-value = ( token / quoted-string ) Consumes a single parameter starting at the beginning of sv, reporting the number of bytes consumed via bytes_used. A non-empty token name, a mandatory "=", and a non-empty token-or-quoted-string value are all required. When

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.
- **allow_bws:** Whether bad whitespace is accepted. Declaration: `bool allow_bws`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1444](../../../../../include/protocol/http/common/helpers.h#L1444)

<a id="method-consume-extension-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_extension_parameter

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_extension_parameter(
      std::string_view sv, std::size_t& bytes_used) noexcept;
```

extension-param = token [ "=" ( token / quoted-string ) ] Consumes a single extension parameter starting at the beginning of sv, reporting the number of bytes consumed via bytes_used. A non-empty token name is mandatory, but unlike consume_parameter() the value is optional: when a "=" follows the name (with no surrounding whitespace) a non-empty

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1485](../../../../../include/protocol/http/common/helpers.h#L1485)

<a id="method-for-each-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> for_each_parameter

**METHOD** · *protocol::http::helpers*

```cpp
template <typename ParameterConsumer>
  static constexpr bool for_each_parameter(std::string_view sv,
                                           bool require_parameter,
                                           ParameterConsumer&& consume);
```

parameters = *( OWS ";" OWS [ parameter ] ) Iterates the ";"-delimited parameters that follow a value (media-type, media-range, transfer-coding, t-codings, expectation, ...), trimming OWS around each ";". A ";" is mandatory before every parameter. When require_parameter is false, empty parameter slots and a trailing ";" are tolerated (as in

**Template parameters:**

- **ParameterConsumer:** `typename ParameterConsumer`.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **require_parameter:** Whether a parameter is required. Declaration: `bool require_parameter`.
- **consume:** Consumer callback for parsed values. Declaration: `ParameterConsumer&& consume`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:1524](../../../../../include/protocol/http/common/helpers.h#L1524)

<a id="method-for-each-forwarded-pair"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> for_each_forwarded_pair

**METHOD** · *protocol::http::helpers*

```cpp
template <typename PairConsumer>
  static constexpr bool for_each_forwarded_pair(std::string_view sv,
                                                PairConsumer&& consume);
```

forwarded-element = [ forwarded-pair ] *( ";" [ forwarded-pair ] ) Iterates the ";"-delimited pairs of a single forwarded-element. Unlike for_each_parameter (RFC 9110 S5.6.6 parameters), no OWS is permitted around ";" and the first pair is not preceded by ";", matching the (a leading ";", consecutive ";;", or a trailing ";") are tolerated and an

**Template parameters:**

- **PairConsumer:** `typename PairConsumer`.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **consume:** Consumer callback for parsed values. Declaration: `PairConsumer&& consume`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:1571](../../../../../include/protocol/http/common/helpers.h#L1571)

<a id="method-for-each-list-element"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> for_each_list_element

**METHOD** · *protocol::http::helpers*

```cpp
template <typename ElementConsumer>
  static constexpr bool for_each_list_element(
      std::string_view sv, ElementConsumer&& consume_element);
```

#element = [ element ] *( OWS "," OWS [ element ] ) Iterates the comma-separated elements of a list-based field value, trimming OWS around each element and skipping empty list elements, as recipients are required to do (RFC 9110 S5.6.1). A comma occurring inside a quoted-string (RFC 9110 S5.6.4) is treated as part of that quoted-string rather than

**Template parameters:**

- **ElementConsumer:** `typename ElementConsumer`.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **consume_element:** Consumer callback for each parsed element. Declaration: `ElementConsumer&& consume_element`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:1610](../../../../../include/protocol/http/common/helpers.h#L1610)

<a id="method-check-auth-param-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_auth_param_list

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_auth_param_list(std::string_view sv) noexcept;
```

Authentication-Info = #auth-param auth-param          = token BWS "=" BWS ( token / quoted-string ) Validates that sv, as a whole, is a "#auth-param" list: a comma- separated sequence of auth-param name/value pairs. This is the shared shape of the Authentication-Info (RFC 9110 S11.6.3) header field value and of the "#auth-param" argument of a

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1666](../../../../../include/protocol/http/common/helpers.h#L1666)

<a id="method-check-credentials"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_credentials

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_credentials(std::string_view sv) noexcept;
```

credentials = auth-scheme [ 1*SP ( token68 / #auth-param ) ] auth-scheme = token auth-param  = token BWS "=" BWS ( token / quoted-string ) Validates that sv, as a whole, is exactly one credentials production, the shared shape of the Authorization (RFC 9110 S11.6.2) and Proxy- Authorization (RFC 9110 S11.7.1) header field values. A mandatory

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1700](../../../../../include/protocol/http/common/helpers.h#L1700)

<a id="method-challenge-opens-param-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> challenge_opens_param_list

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool challenge_opens_param_list(
      std::string_view element) noexcept;
```

Given a single, already-trimmed challenge element that is known to be a valid challenge (i.e. check_credentials(element) is true), reports whether that challenge opens a "#auth-param" list, meaning that later comma-separated auth-param elements may still belong to it. This is the case only when the challenge has a "1*SP" argument region whose

**Parameters:**

- **element:** Element being parsed or matched. Declaration: `std::string_view element`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1729](../../../../../include/protocol/http/common/helpers.h#L1729)

<a id="method-check-challenge-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_challenge_list

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_challenge_list(std::string_view sv) noexcept;
```

WWW-Authenticate  = #challenge Proxy-Authenticate = #challenge challenge         = auth-scheme [ 1*SP ( token68 / #auth-param ) ] auth-scheme       = token auth-param        = token BWS "=" BWS ( token / quoted-string ) Validates that sv, as a whole, is a "#challenge" list, the shared shape of the WWW-Authenticate (RFC 9110 S11.6.1) and

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1767](../../../../../include/protocol/http/common/helpers.h#L1767)

<a id="method-consume-product"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_product

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr std::string_view consume_product(
      std::string_view sv) noexcept;
```

product         = token [ "/" product-version ] product-version = token Consumes a single product from the front of sv and returns the consumed prefix, or an empty string_view if sv does not begin with a product. A product is a non-empty token, optionally followed by a "/" and a non- empty product-version token; a trailing "/" with no version is

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1801](../../../../../include/protocol/http/common/helpers.h#L1801)

<a id="method-check-product-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_product_list

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_product_list(std::string_view sv) noexcept;
```

product-list = product *( RWS ( product / comment ) ) RWS          = 1*( SP / HTAB ) Validates that sv, as a whole, is exactly one product list, the shared shape of the User-Agent (RFC 9110 S10.1.5) and Server (RFC 9110 S10.2.4) header field values. The value MUST begin with a product, so an empty value, a leading comment, or leading whitespace is

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1832](../../../../../include/protocol/http/common/helpers.h#L1832)

<a id="method-check-uri-host"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_host

**METHOD** · *protocol::http::helpers*

```cpp
static host_type check_uri_host(std::string_view sv);
```

uri-host       | IP-literal / IPv4address / reg-name

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A value of type `host_type`.

**Source:** [include/protocol/http/common/helpers.h:1863](../../../../../include/protocol/http/common/helpers.h#L1863)

<a id="method-is-uri-scheme"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_uri_scheme

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool is_uri_scheme(std::string_view sv) noexcept;
```

scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) Validates that sv, as a whole, is a bare URI scheme name (no trailing ":"). This is the shared shape of a URI scheme (RFC 3986 S3.1), of the "proto" parameter value of the Forwarded header (RFC 7239 S5.4), and of an X-Forwarded-Proto list element. Returns false on an empty value, a non-ALPHA

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1889](../../../../../include/protocol/http/common/helpers.h#L1889)

<a id="method-consume-uri-scheme"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_uri_scheme

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_uri_scheme(std::string_view sv,
                                           std::size_t& i) noexcept;
```

scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) Detects an optional leading "scheme ':'" prefix. On success, advances 'i' past the ':' and returns true; otherwise leaves 'i' untouched and returns false (the value is then treated as a relative reference). The scheme portion is validated with is_uri_scheme.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **i:** Zero-based index of the requested element. Declaration: `std::size_t& i`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1911](../../../../../include/protocol/http/common/helpers.h#L1911)

<a id="method-consume-uri-pct-encoded"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_uri_pct_encoded

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool consume_uri_pct_encoded(std::string_view sv,
                                                std::size_t& i) noexcept;
```

pct-encoded = "%" HEXDIG HEXDIG Assumes sv[i] == '%'. On success, advances 'i' past the two HEXDIGs and returns true; otherwise returns false.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **i:** Zero-based index of the requested element. Declaration: `std::size_t& i`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1930](../../../../../include/protocol/http/common/helpers.h#L1930)

<a id="method-percent-decode-validate"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> percent_decode_validate

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool percent_decode_validate(std::string_view sv) noexcept;
```

Read-only counterpart of 'percent_decode_in_place'. Scans every well-formed "%HH" pct-encoded triplet within 'sv' and checks that its decoded byte is never NUL (0x00), which is never valid within a path, without writing back into (or otherwise mutating) the source buffer. Assumes 'sv' was already validated as syntactically well-formed (every '%'

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1951](../../../../../include/protocol/http/common/helpers.h#L1951)

<a id="method-percent-decode-in-place"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> percent_decode_in_place

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr void percent_decode_in_place(
      std::string_view& sv) noexcept;
```

Decodes every well-formed "%HH" pct-encoded triplet within 'sv' back into its raw byte, writing the result back into the very same underlying buffer and shrinking 'sv' to the decoded length (mirroring how ows_ltrim/ows_rtrim mutate their string_view argument in place). This is safe because a decoded string is never longer than its encoded source,

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view& sv`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:1997](../../../../../include/protocol/http/common/helpers.h#L1997)

<a id="method-check-uri-userinfo"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_userinfo

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_uri_userinfo(std::string_view sv) noexcept;
```

userinfo = *( unreserved / pct-encoded / sub-delims / ":" )

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2037](../../../../../include/protocol/http/common/helpers.h#L2037)

<a id="method-check-host-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_host_port

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_host_port(std::string_view sv,
                                        std::string_view& uri_host,
                                        std::string_view& port,
                                        host_type& type) noexcept;
```

host = IP-literal / IPv4address / reg-name port = *DIGIT Validates that sv, as a whole, is a uri-host optionally followed by a ":" port. This is the userinfo-free portion of an authority, shared by the Host header (RFC 9110 S7.2), an authority-form request-target, and the serialized-origin of the Origin header (RFC 6454 S7). An IP-literal host

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **uri_host:** Host component of the URI. Declaration: `std::string_view& uri_host`.
- **port:** Network port value. Declaration: `std::string_view& port`.
- **type:** Type or classification to store or inspect. Declaration: `host_type& type`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2076](../../../../../include/protocol/http/common/helpers.h#L2076)

<a id="method-check-host-port-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_host_port

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_host_port(std::string_view sv) noexcept;
```

Checks host port.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2094](../../../../../include/protocol/http/common/helpers.h#L2094)

<a id="method-check-serialized-origin"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_serialized_origin

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_serialized_origin(std::string_view sv) noexcept;
```

serialized-origin = scheme "://" host [ ":" port ] Validates that sv, as a whole, is a single serialized origin: a URI scheme, the literal "://", and a userinfo-free "host [ ":" port ]". A serialized origin has no path, query, fragment, trailing slash, or userinfo. This is the shared shape of an Origin list element (RFC 6454 S7) and of the

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2115](../../../../../include/protocol/http/common/helpers.h#L2115)

<a id="method-check-uri-authority"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_authority

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_uri_authority(std::string_view sv) noexcept;
```

authority = [ userinfo "@" ] host [ ":" port ]

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2132](../../../../../include/protocol/http/common/helpers.h#L2132)

<a id="method-check-uri-reference"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_reference

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool check_uri_reference(std::string_view sv,
                                            bool allow_fragment) noexcept;
```

URI-reference = URI / relative-ref URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ] relative-ref  = relative-part [ "?" query ] [ "#" fragment ] hier-part     = "//" authority path-abempty / path-absolute / path-rootless / path-empty relative-part = "//" authority path-abempty / path-absolute / path-noscheme / path-empty query

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **allow_fragment:** Whether URI fragments are allowed. Declaration: `bool allow_fragment`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2163](../../../../../include/protocol/http/common/helpers.h#L2163)

<a id="method-split-authority-host-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> split_authority_host_port

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool split_authority_host_port(
      std::string_view sv, std::string_view& decoded_uri_host,
      std::size_t& colon_found_at) noexcept;
```

Splits a "uri-host [ ':' port ]" sequence into its uri-host substring and the position of the uri-host/port separator ':', recognizing IP-literal hosts (e.g. "[::1]") so that colons nested inside "[...]" are never mistaken for that separator. Returns false only when sv begins with an unterminated IP-literal (an opening "[" with no matching "]");

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **decoded_uri_host:** Output decoded URI host. Declaration: `std::string_view& decoded_uri_host`.
- **colon_found_at:** Output position of the colon separator. Declaration: `std::size_t& colon_found_at`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2261](../../../../../include/protocol/http/common/helpers.h#L2261)

<a id="method-try-to-deserialize-as-authority-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_authority_form

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr deserialization_status try_to_deserialize_as_authority_form(
      std::string_view sv, std::string_view& uri_host, std::string_view& port,
      host_type& type, std::size_t& bytes_used) noexcept;
```

authority-form | uri-host ":" port uri-host       | IP-literal / IPv4address / reg-name port           | *DIGIT IP-literal     | "[" ( IPv6address / IPvFuture ) "]" IPvFuture      | "v" 1*HEXDIG "." | 1*( unreserved / sub-delims / ":" ) IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet dec-octet      | DIGIT | / %x31-39 DIGIT |

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **uri_host:** Host component of the URI. Declaration: `std::string_view& uri_host`.
- **port:** Network port value. Declaration: `std::string_view& port`.
- **type:** Type or classification to store or inspect. Declaration: `host_type& type`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.

**Returns:** A value of type `deserialization_status`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2307](../../../../../include/protocol/http/common/helpers.h#L2307)

<a id="method-try-to-deserialize-as-asterisk-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_asterisk_form

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr deserialization_status try_to_deserialize_as_asterisk_form(
      std::string_view sv, std::size_t& bytes_used) noexcept;
```

asterisk-form  | "*"

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.

**Returns:** A value of type `deserialization_status`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2351](../../../../../include/protocol/http/common/helpers.h#L2351)

<a id="method-try-to-deserialize-as-origin-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_origin_form

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr deserialization_status try_to_deserialize_as_origin_form(
      std::string_view sv, std::string_view& consumed_path,
      std::string_view& consumed_query, std::size_t& bytes_used) noexcept;
```

origin-form    | absolute-path [ "?" query ]

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **consumed_path:** Output view of the consumed path. Declaration: `std::string_view& consumed_path`.
- **consumed_query:** Output view of the consumed query. Declaration: `std::string_view& consumed_query`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.

**Returns:** A value of type `deserialization_status`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2367](../../../../../include/protocol/http/common/helpers.h#L2367)

<a id="method-default-port-for-scheme"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> default_port_for_scheme

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr std::string_view default_port_for_scheme(
      std::string_view scheme) noexcept;
```

to when the authority omits one ("80" for "http", "443" for "https", matched case-insensitively). Any other (or empty) scheme has no known default and yields an empty view.

**Parameters:**

- **scheme:** URI or authentication scheme. Declaration: `std::string_view scheme`.

**Returns:** A value of type `std::string_view`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2399](../../../../../include/protocol/http/common/helpers.h#L2399)

<a id="method-ports-equivalent"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ports_equivalent

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool ports_equivalent(std::string_view scheme,
                                         std::string_view a,
                                         std::string_view b) noexcept;
```

True when two port views denote the same effective port for "scheme". Equal raw digit runs are always equivalent. Otherwise, an omitted (empty ) port is normalized to the scheme default before comparing, so e.g. "http" with "" and "80" compare equal while "" and "8080" do not. When the scheme has no known default, only exact string equality

**Parameters:**

- **scheme:** URI or authentication scheme. Declaration: `std::string_view scheme`.
- **a:** Text value to compare. Declaration: `std::string_view a`.
- **b:** Byte or text value to process. Declaration: `std::string_view b`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2414](../../../../../include/protocol/http/common/helpers.h#L2414)

<a id="method-try-to-deserialize-as-absolute-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_absolute_form

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr deserialization_status try_to_deserialize_as_absolute_form(
      std::string_view sv, std::string_view& consumed_path,
      std::string_view& consumed_query, bool& has_authority,
      std::string_view& uri_host, std::string_view& port, host_type& type,
      std::string_view& scheme, std::size_t& bytes_used) noexcept;
```

absolute-form  | absolute-URI absolute-URI   | scheme ":" hier-part [ "?" query ] scheme         | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) hier-part      | "//" authority path-abempty | / path-absolute | / path-rootless | / path-empty authority      | [ userinfo "@" ] host [ ":" port ] userinfo       | *( unreserved / pct-encoded / sub-delims /

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **consumed_path:** Output view of the consumed path. Declaration: `std::string_view& consumed_path`.
- **consumed_query:** Output view of the consumed query. Declaration: `std::string_view& consumed_query`.
- **has_authority:** Output flag indicating an authority component. Declaration: `bool& has_authority`.
- **uri_host:** Host component of the URI. Declaration: `std::string_view& uri_host`.
- **port:** Network port value. Declaration: `std::string_view& port`.
- **type:** Type or classification to store or inspect. Declaration: `host_type& type`.
- **scheme:** URI or authentication scheme. Declaration: `std::string_view& scheme`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.

**Returns:** A value of type `deserialization_status`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2463](../../../../../include/protocol/http/common/helpers.h#L2463)

<a id="method-try-to-deserialize-as-absolute-path"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_absolute_path

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr deserialization_status try_to_deserialize_as_absolute_path(
      std::string_view sv, std::string_view& consumed_path,
      std::size_t& bytes_used) noexcept;
```

absolute-path  | 1*( "/" segment ) segment        | *pchar pchar          | unreserved / pct-encoded / sub-delims / ":" / "@" unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~" pct-encoded    | "%" HEXDIG HEXDIG sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / | "," / ";" / "="

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **consumed_path:** Output view of the consumed path. Declaration: `std::string_view& consumed_path`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.

**Returns:** A value of type `deserialization_status`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2625](../../../../../include/protocol/http/common/helpers.h#L2625)

<a id="method-try-to-deserialize-as-query"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_query

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr deserialization_status try_to_deserialize_as_query(
      std::string_view sv, std::string_view& consumed_query,
      std::size_t& bytes_used) noexcept;
```

query          | *( pchar / "/" / "?" ) pchar          | unreserved / pct-encoded / sub-delims / ":" / "@" unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~" pct-encoded    | "%" HEXDIG HEXDIG sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / | "," / ";" / "="

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **consumed_query:** Output view of the consumed query. Declaration: `std::string_view& consumed_query`.
- **bytes_used:** Output byte count updated by this operation. Declaration: `std::size_t& bytes_used`.

**Returns:** A value of type `deserialization_status`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2665](../../../../../include/protocol/http/common/helpers.h#L2665)

<a id="method-iequals"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> iequals

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr bool iequals(std::string_view a, std::string_view b);
```

Compares two string_views for case-insensitive equality.

**Parameters:**

- **a:** Text value to compare. Declaration: `std::string_view a`.
- **b:** Byte or text value to process. Declaration: `std::string_view b`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/helpers.h:2693](../../../../../include/protocol/http/common/helpers.h#L2693)

<a id="method-split-query-parameters"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> split_query_parameters

**METHOD** · *protocol::http::helpers*

```cpp
static constexpr std::size_t split_query_parameters(
      std::string_view query, std::span<std::string_view> out_keys,
      std::span<std::string_view> out_values) noexcept;
```

Splits a raw query string (the query component already stripped of its leading '?') into key/value pairs following the application/x-www-form- urlencoded convention: query      = pair *( "&" pair ) pair       = key [ "=" value ] key/value  = *( any char except "&" ; value also except first "=" ) Rules: pairs are separated by '&'; the first '='

**Parameters:**

- **query:** URI query text. Declaration: `std::string_view query`.
- **out_keys:** Output span for parsed keys. Declaration: `std::span<std::string_view> out_keys`.
- **out_values:** Output span for parsed values. Declaration: `std::span<std::string_view> out_values`.

**Returns:** The split query parameters count or size.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/common/helpers.h:2717](../../../../../include/protocol/http/common/helpers.h#L2717)
