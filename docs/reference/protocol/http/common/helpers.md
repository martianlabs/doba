<a id="helpers"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/helpers/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/helpers/title.svg" alt="helpers">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/helpers/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/helpers/title-dark.svg" alt="helpers">
</a>

**Header:** [include/protocol/http/common/helpers.h](../../../../../include/protocol/http/common/helpers.h)

Internal grammar helpers used by HTTP parsing and header checks. Character tests use byte values and ASCII rules, not locales or Unicode. Unless stated otherwise, returned views borrow the input and no text is copied.

<a id="api"></a>
<a id="type-helpers"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> helpers

`martianlabs::doba::protocol::http::helpers` groups static parsing operations. Prefix consumers do not establish that a whole HTTP field or request is valid.

<a id="enum-host-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> host_type

```cpp
enum class host_type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
```

Classifies a host as invalid, bracketed IP literal, IPv4, or registered name.

<a id="method-is-digit"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_digit

```cpp
static constexpr bool is_digit(unsigned char c) noexcept;
```

Returns whether the byte is an ASCII decimal digit.

<a id="method-is-digit-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_digit

```cpp
static constexpr bool is_digit(char c) noexcept;
```

Returns whether the byte is an ASCII decimal digit.

<a id="method-is-digits"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_digits

```cpp
static constexpr bool is_digits(std::string_view sv) noexcept;
```

Requires at least one ASCII decimal digit and no other bytes.

<a id="method-parse-size-t"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parse_size_t

```cpp
static constexpr bool parse_size_t(std::string_view sv,
                                     std::size_t& out) noexcept;
```

Parses the whole nonempty decimal string into `out`, checking `size_t` overflow. Returns false and leaves `out` unchanged on failure; leading zeroes are allowed.

<a id="method-is-hex-digit"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_hex_digit

```cpp
static constexpr bool is_hex_digit(unsigned char c) noexcept;
```

Recognizes ASCII `0-9`, `A-F`, and `a-f`.

<a id="method-is-hex-digit-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_hex_digit

```cpp
static constexpr bool is_hex_digit(char c) noexcept;
```

Recognizes ASCII `0-9`, `A-F`, and `a-f`.

<a id="method-is-alpha"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_alpha

```cpp
static constexpr bool is_alpha(unsigned char c) noexcept;
```

Recognizes ASCII letters only.

<a id="method-is-alpha-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_alpha

```cpp
static constexpr bool is_alpha(char c) noexcept;
```

Recognizes ASCII letters only.

<a id="method-is-token"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token

```cpp
static constexpr bool is_token(unsigned char c) noexcept;
```

The byte overloads recognize HTTP token characters. The string overload requires a nonempty string made entirely of those characters.

<a id="method-is-token-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token

```cpp
static constexpr bool is_token(char c) noexcept;
```

The byte overloads recognize HTTP token characters. The string overload requires a nonempty string made entirely of those characters.

<a id="method-is-token-3"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token

```cpp
static constexpr bool is_token(std::string_view sv) noexcept;
```

The byte overloads recognize HTTP token characters. The string overload requires a nonempty string made entirely of those characters.

<a id="method-is-unreserved"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_unreserved

```cpp
static constexpr bool is_unreserved(unsigned char c) noexcept;
```

Recognizes ASCII letters, digits, and `-._~`.

<a id="method-is-unreserved-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_unreserved

```cpp
static constexpr bool is_unreserved(char c) noexcept;
```

Recognizes ASCII letters, digits, and `-._~`.

<a id="method-is-sub-delim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_sub_delim

```cpp
static constexpr bool is_sub_delim(unsigned char c) noexcept;
```

Recognizes URI sub-delimiters: `!$&'()*+,;=`.

<a id="method-is-sub-delim-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_sub_delim

```cpp
static constexpr bool is_sub_delim(char c) noexcept;
```

Recognizes URI sub-delimiters: `!$&'()*+,;=`.

<a id="method-is-pchar"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_pchar

```cpp
static constexpr bool is_pchar(unsigned char c) noexcept;
```

Recognizes unreserved bytes, sub-delimiters, `:`, and `@`. A percent escape is handled separately.

<a id="method-is-pchar-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_pchar

```cpp
static constexpr bool is_pchar(char c) noexcept;
```

Recognizes unreserved bytes, sub-delimiters, `:`, and `@`. A percent escape is handled separately.

<a id="method-is-vchar"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_vchar

```cpp
static constexpr bool is_vchar(unsigned char c) noexcept;
```

Recognizes visible ASCII bytes `0x21-0x7e`.

<a id="method-is-vchar-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_vchar

```cpp
static constexpr bool is_vchar(char c) noexcept;
```

Recognizes visible ASCII bytes `0x21-0x7e`.

<a id="method-is-qdtext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_qdtext

```cpp
static constexpr bool is_qdtext(unsigned char c) noexcept;
```

Recognizes unescaped quoted-string text: tab, space, visible bytes other than double quote/backslash, and bytes `0x80-0xff`.

<a id="method-is-qdtext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_qdtext

```cpp
static constexpr bool is_qdtext(char c) noexcept;
```

Recognizes unescaped quoted-string text: tab, space, visible bytes other than double quote/backslash, and bytes `0x80-0xff`.

<a id="method-is-obs-text"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_obs_text

```cpp
static constexpr bool is_obs_text(unsigned char c) noexcept;
```

Recognizes bytes `0x80-0xff`, without decoding a character set.

<a id="method-is-obs-text-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_obs_text

```cpp
static constexpr bool is_obs_text(char c) noexcept;
```

Recognizes bytes `0x80-0xff`, without decoding a character set.

<a id="method-is-ctext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ctext

```cpp
static constexpr bool is_ctext(unsigned char c) noexcept;
```

Recognizes comment text: tab, space, visible bytes other than parentheses/backslash, and bytes `0x80-0xff`.

<a id="method-is-ctext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ctext

```cpp
static constexpr bool is_ctext(char c) noexcept;
```

Recognizes comment text: tab, space, visible bytes other than parentheses/backslash, and bytes `0x80-0xff`.

<a id="method-is-atext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_atext

```cpp
static constexpr bool is_atext(unsigned char c) noexcept;
```

Recognizes the ASCII letters, digits, and punctuation allowed in a mailbox atom.

<a id="method-is-atext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_atext

```cpp
static constexpr bool is_atext(char c) noexcept;
```

Recognizes the ASCII letters, digits, and punctuation allowed in a mailbox atom.

<a id="method-is-dtext"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dtext

```cpp
static constexpr bool is_dtext(unsigned char c) noexcept;
```

Recognizes domain-literal text: bytes `0x21-0x5a`, `0x5e-0x7e`, and `0x80-0xff`.

<a id="method-is-dtext-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dtext

```cpp
static constexpr bool is_dtext(char c) noexcept;
```

Recognizes domain-literal text: bytes `0x21-0x5a`, `0x5e-0x7e`, and `0x80-0xff`.

<a id="method-is-etagc"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_etagc

```cpp
static constexpr bool is_etagc(unsigned char c) noexcept;
```

Recognizes entity-tag content: visible bytes except double quote, plus bytes `0x80-0xff`.

<a id="method-is-etagc-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_etagc

```cpp
static constexpr bool is_etagc(char c) noexcept;
```

Recognizes entity-tag content: visible bytes except double quote, plus bytes `0x80-0xff`.

<a id="method-is-cookie-octet"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_octet

```cpp
static constexpr bool is_cookie_octet(unsigned char c) noexcept;
```

Recognizes cookie-value octets, excluding whitespace, double quote, comma, semicolon, backslash, and non-ASCII bytes.

<a id="method-is-cookie-octet-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_octet

```cpp
static constexpr bool is_cookie_octet(char c) noexcept;
```

Recognizes cookie-value octets, excluding whitespace, double quote, comma, semicolon, backslash, and non-ASCII bytes.

<a id="method-is-cookie-av-octet"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_av_octet

```cpp
static constexpr bool is_cookie_av_octet(unsigned char c) noexcept;
```

Recognizes attribute text: ASCII space through `~`, except semicolon.

<a id="method-is-cookie-av-octet-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_av_octet

```cpp
static constexpr bool is_cookie_av_octet(char c) noexcept;
```

Recognizes attribute text: ASCII space through `~`, except semicolon.

<a id="method-is-entity-tag"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_entity_tag

```cpp
static constexpr bool is_entity_tag(std::string_view sv) noexcept;
```

Checks a complete quoted entity tag with an optional uppercase `W/` prefix. Backslashes are ordinary tag bytes, not quote escapes.

<a id="method-is-ows"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ows

```cpp
static constexpr bool is_ows(unsigned char c) noexcept;
```

Recognizes space or tab only.

<a id="method-is-ows-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ows

```cpp
static constexpr bool is_ows(char c) noexcept;
```

Recognizes space or tab only.

<a id="method-is-qvalue"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_qvalue

```cpp
static constexpr bool is_qvalue(std::string_view sv) noexcept;
```

Accepts `0` or `1`, optionally followed by a dot and up to three digits. A fraction after `1` must contain only zeroes. `0.` and `1.` are accepted.

<a id="method-consume-weight"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_weight

```cpp
static constexpr bool consume_weight(std::string_view sv) noexcept;
```

Checks the whole suffix as optional whitespace, `;`, optional whitespace, and case-insensitive `q=` followed by a qvalue. Whitespace around `=` or after the value is rejected.

<a id="method-tolower-ascii"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tolower_ascii

```cpp
static constexpr unsigned char tolower_ascii(unsigned char c) noexcept;
```

Maps ASCII `A-Z` to `a-z`; leaves all other bytes unchanged.

<a id="method-tolower-ascii-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tolower_ascii

```cpp
static constexpr char tolower_ascii(char c) noexcept;
```

Maps ASCII `A-Z` to `a-z`; leaves all other bytes unchanged.

<a id="method-ows-ltrim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ows_ltrim

```cpp
static constexpr void ows_ltrim(std::string_view& sv) noexcept;
```

Removes leading spaces/tabs by changing the supplied view. The backing bytes are untouched.

<a id="method-ows-rtrim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ows_rtrim

```cpp
static constexpr void ows_rtrim(std::string_view& sv) noexcept;
```

Removes trailing spaces/tabs by changing the supplied view. The backing bytes are untouched.

<a id="method-ows-trim"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ows_trim

```cpp
static constexpr void ows_trim(std::string_view& sv) noexcept;
```

Removes leading and trailing spaces/tabs by changing the supplied view. The backing bytes are untouched.

<a id="method-skip-n-hextets"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> skip_n_hextets

```cpp
static constexpr int skip_n_hextets(std::string_view sv);
```

Counts complete colon-separated IPv6 hextets, each one to four hexadecimal digits. Returns -1 for malformed input, including empty input or an empty segment.

<a id="method-is-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_port

```cpp
static constexpr bool is_port(std::string_view sv);
```

Accepts empty text or any number of ASCII decimal digits. It does not convert the port or limit it to 65535.

<a id="method-is-dec-octet"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dec_octet

```cpp
static constexpr bool is_dec_octet(std::string_view sv, std::size_t digits,
                                     std::size_t off);
```

Checks a decimal octet from 0 to 255 without leading zeroes on multi-digit values. The overload with `digits` and `off` expects an already digit-checked substring of that length starting at `off`; those bounds are a caller precondition.

<a id="method-is-dec-octet-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_dec_octet

```cpp
static constexpr bool is_dec_octet(std::string_view sv) noexcept;
```

Checks a decimal octet from 0 to 255 without leading zeroes on multi-digit values. The overload with `digits` and `off` expects an already digit-checked substring of that length starting at `off`; those bounds are a caller precondition.

<a id="method-is-ip-v-future"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_v_future

```cpp
static constexpr bool is_ip_v_future(std::string_view sv);
```

Checks an IPvFuture literal without brackets: `v`/`V`, hexadecimal version, dot, and nonempty allowed address text.

<a id="method-is-ip-v6-address"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_v6_address

```cpp
static constexpr bool is_ip_v6_address(std::string_view sv);
```

Checks an unbracketed IPv6 address, including a compressed run and an optional IPv4 tail. Zone identifiers are not supported.

<a id="method-is-ip-v4-address"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_v4_address

```cpp
static constexpr bool is_ip_v4_address(std::string_view sv);
```

Checks four decimal octets from 0 to 255, with dots and no multi-digit leading zeroes.

<a id="method-is-ip-literal"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_ip_literal

```cpp
static constexpr bool is_ip_literal(std::string_view sv);
```

Requires brackets around an IPv6 or IPvFuture address.

<a id="method-is-reg-name"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_reg_name

```cpp
static constexpr bool is_reg_name(std::string_view sv);
```

Checks a URI registered name made of unreserved bytes, sub-delimiters, and `%HH` escapes. Empty text is accepted; escapes are not decoded.

<a id="method-is-token68"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_token68

```cpp
static constexpr bool is_token68(std::string_view sv) noexcept;
```

Checks a nonempty token68 alphabet followed by optional `=` padding. This is not a Base64 decoder or a padding-length check.

<a id="method-is-base64-char"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_base64_char

```cpp
static constexpr bool is_base64_char(char c) noexcept;
```

Recognizes ASCII Base64 alphabet bytes: letters, digits, `+`, and `/`.

<a id="method-check-base64-value"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_base64_value

```cpp
static constexpr bool check_base64_value(std::string_view sv) noexcept;
```

Requires nonempty Base64 text whose length is divisible by four, with at most two `=` bytes in the final padding positions. It does not check unused padding bits or decoded length.

<a id="method-consume-token"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_token

```cpp
static constexpr std::string_view consume_token(
      std::string_view sv) noexcept;
```

Returns the longest token prefix, or an empty view when no token starts here. The input view and bytes are unchanged.

<a id="method-consume-quoted-string"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_quoted_string

```cpp
static constexpr std::string_view consume_quoted_string(
      std::string_view sv) noexcept;
```

Returns a complete quoted-string prefix, including its quotes and escapes, or an empty view on failure. It validates quoted pairs but does not unescape them.

<a id="method-consume-comment"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_comment

```cpp
static constexpr std::string_view consume_comment(
      std::string_view sv) noexcept;
```

Returns a complete balanced comment prefix, including nested comments and supported escapes, or an empty view on failure. It does not remove delimiters or escapes.

<a id="method-consume-cfws"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_cfws

```cpp
static constexpr bool consume_cfws(std::string_view sv,
                                     std::size_t& i) noexcept;
```

Advances `i` past spaces, tabs, and balanced comments. Returns false on a malformed comment. Start with `i <= sv.size()`; failure can leave `i` advanced.

<a id="method-consume-dot-atom"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_dot_atom

```cpp
static constexpr bool consume_dot_atom(std::string_view sv,
                                         std::size_t& i) noexcept;
```

Advances `i` over a nonempty dot-separated mailbox atom and surrounding comments/whitespace. Empty segments fail. Failure can leave `i` advanced.

<a id="method-consume-domain-literal"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_domain_literal

```cpp
static constexpr bool consume_domain_literal(std::string_view sv,
                                               std::size_t& i) noexcept;
```

Advances `i` over a bracketed domain literal and surrounding comments/whitespace. Its contents are not checked as an IP address; failure can leave `i` advanced.

<a id="method-consume-addr-spec"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_addr_spec

```cpp
static constexpr bool consume_addr_spec(std::string_view sv,
                                          std::size_t& i) noexcept;
```

Advances `i` over a mailbox local part, `@`, and domain, with the supported comments/quoted forms. Failure can leave `i` advanced.

<a id="method-consume-word"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_word

```cpp
static constexpr bool consume_word(std::string_view sv,
                                     std::size_t& i) noexcept;
```

Advances `i` over a mailbox atom or quoted string and surrounding comments/whitespace. Failure can leave `i` advanced.

<a id="method-check-mailbox"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_mailbox

```cpp
static constexpr bool check_mailbox(std::string_view sv) noexcept;
```

Checks a complete address or display-name/angle-address mailbox. It validates structure without resolving domains or verifying delivery.

<a id="method-consume-token-or-quoted-string"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_token_or_quoted_string

```cpp
static constexpr std::string_view consume_token_or_quoted_string(
      std::string_view sv) noexcept;
```

Returns a quoted-string prefix if the first byte is a quote; otherwise returns a token prefix. An empty view means failure.

<a id="method-is-cookie-value"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_value

```cpp
static constexpr bool is_cookie_value(std::string_view sv) noexcept;
```

Accepts cookie octets, optionally enclosed in double quotes. Empty values are accepted. Quotes do not enable escaping or extra characters.

<a id="method-is-cookie-pair"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_pair

```cpp
static constexpr bool is_cookie_pair(std::string_view sv) noexcept;
```

Checks a nonempty token name, `=`, and a cookie value. Empty values are accepted.

<a id="method-is-cookie-av"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_cookie_av

```cpp
static constexpr bool is_cookie_av(std::string_view sv) noexcept;
```

Requires nonempty cookie-attribute text. Attribute names and values are not interpreted.

<a id="method-is-directive"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_directive

```cpp
static constexpr bool is_directive(std::string_view sv) noexcept;
```

Checks a whole token directive with an optional `=token` or `="quoted value"`, without whitespace around `=`.

<a id="method-consume-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_parameter

```cpp
static constexpr bool consume_parameter(std::string_view sv,
                                          std::size_t& bytes_used,
                                          bool allow_bws) noexcept;
```

Consumes a `name=value` prefix and writes its length to `bytes_used`. With `allow_bws`, spaces/tabs around `=` are accepted. Returns false with `bytes_used=0` on failure.

<a id="method-consume-extension-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_extension_parameter

```cpp
static constexpr bool consume_extension_parameter(
      std::string_view sv, std::size_t& bytes_used) noexcept;
```

Consumes a token prefix with an optional `=token` or `="quoted value"`. Writes the consumed length, or zero on failure. Whitespace around `=` is not consumed.

<a id="method-for-each-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> for_each_parameter

```cpp
template <typename ParameterConsumer>
  static constexpr bool for_each_parameter(std::string_view sv,
                                           bool require_parameter,
                                           ParameterConsumer&& consume);
```

Walks semicolon-prefixed parameters. Calls `consume(remaining, bytes)` for each nonempty slot; the callback must return true and consume between one and the remaining number of bytes. `require_parameter` rejects empty slots, not an entirely empty input. Callback exceptions propagate.

<a id="method-for-each-forwarded-pair"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> for_each_forwarded_pair

```cpp
template <typename PairConsumer>
  static constexpr bool for_each_forwarded_pair(std::string_view sv,
                                                PairConsumer&& consume);
```

Walks semicolon-separated forwarded pairs without trimming whitespace. Empty slots are skipped. Each callback receives the remaining suffix and must report a positive in-bounds consumed length; false stops parsing. Callback exceptions propagate.

<a id="method-for-each-list-element"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> for_each_list_element

```cpp
template <typename ElementConsumer>
  static constexpr bool for_each_list_element(
      std::string_view sv, ElementConsumer&& consume_element);
```

Walks comma-separated elements while respecting quoted strings and quoted escapes. Empty elements are skipped. Right-trims spaces/tabs before commas and left-trims after commas; the first leading and final trailing whitespace are not generally trimmed. Unbalanced quotes or backslashes outside quotes fail. The callback receives borrowed views; false stops parsing and exceptions propagate.

<a id="method-check-auth-param-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_auth_param_list

```cpp
static constexpr bool check_auth_param_list(std::string_view sv) noexcept;
```

Checks a comma-separated list of complete `name=value` authentication parameters, allowing whitespace around `=`. Empty lists are accepted.

<a id="method-check-credentials"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_credentials

```cpp
static constexpr bool check_credentials(std::string_view sv) noexcept;
```

Checks a scheme token alone, or followed by spaces and token68/authentication parameters. It does not decode or verify credentials.

<a id="method-challenge-opens-param-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> challenge_opens_param_list

```cpp
static constexpr bool challenge_opens_param_list(
      std::string_view element) noexcept;
```

Detects whether text following the initial scheme is not token68. Used after a challenge has passed validation; it is not a standalone validator.

<a id="method-check-challenge-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_challenge_list

```cpp
static constexpr bool check_challenge_list(std::string_view sv) noexcept;
```

Checks challenges and following authentication parameters, tracking the preceding scheme. Empty lists are accepted.

<a id="method-consume-product"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_product

```cpp
static constexpr std::string_view consume_product(
      std::string_view sv) noexcept;
```

Returns a product-token prefix with an optional `/version` token, or an empty view on failure.

<a id="method-check-product-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_product_list

```cpp
static constexpr bool check_product_list(std::string_view sv) noexcept;
```

Requires a first product, then whitespace-separated products or balanced comments. Trailing whitespace is rejected.

<a id="method-check-uri-host"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_host

```cpp
static host_type check_uri_host(std::string_view sv);
```

Classifies an IP literal, strict IPv4 address, or registered name. Empty text is a registered name. Text that fails strict IPv4 may still be a valid registered name.

<a id="method-is-uri-scheme"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> is_uri_scheme

```cpp
static constexpr bool is_uri_scheme(std::string_view sv) noexcept;
```

Requires an ASCII letter, followed by ASCII letters, digits, `+`, `-`, or `.`.

<a id="method-consume-uri-scheme"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_uri_scheme

```cpp
static constexpr bool consume_uri_scheme(std::string_view sv,
                                           std::size_t& i) noexcept;
```

Checks the prefix before the first colon as a scheme and sets `i` just after that colon. It starts at the beginning of `sv`, regardless of the incoming `i`. Failure leaves `i` unchanged.

<a id="method-consume-uri-pct-encoded"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> consume_uri_pct_encoded

```cpp
static constexpr bool consume_uri_pct_encoded(std::string_view sv,
                                                std::size_t& i) noexcept;
```

Expects `i` to point at a percent sign with two following hexadecimal digits. Checks those digits and advances `i` by three; it does not check the percent sign itself. Failure leaves `i` unchanged.

<a id="method-percent-decode-validate"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> percent_decode_validate

```cpp
static constexpr bool percent_decode_validate(std::string_view sv) noexcept;
```

Returns false if a well-formed `%HH` triplet decodes to NUL. It does not reject malformed escapes or literal NUL bytes; URI syntax must be checked separately.

<a id="method-percent-decode-in-place"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> percent_decode_in_place

```cpp
static constexpr void percent_decode_in_place(
      std::string_view& sv) noexcept;
```

Decodes well-formed `%HH` triplets into the backing buffer and shortens `sv`. The buffer must be writable despite the `string_view` interface. Malformed escapes and `+` are left unchanged; `%00` becomes NUL, so validate first when NUL is forbidden.

<a id="method-check-uri-userinfo"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_userinfo

```cpp
static constexpr bool check_uri_userinfo(std::string_view sv) noexcept;
```

Checks unreserved bytes, sub-delimiters, colons, and `%HH` escapes. Empty text is accepted.

<a id="method-check-host-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_host_port

```cpp
static constexpr bool check_host_port(std::string_view sv,
                                        std::string_view& uri_host,
                                        std::string_view& port,
                                        host_type& type) noexcept;
```

Checks a whole host with an optional colon/port and produces borrowed host/port views and a host type in the output overload. Empty hosts and empty decimal ports are accepted; numbers are not range-checked. Output arguments may be changed on failure.

<a id="method-check-host-port-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_host_port

```cpp
static constexpr bool check_host_port(std::string_view sv) noexcept;
```

Checks a whole host with an optional colon/port and produces borrowed host/port views and a host type in the output overload. Empty hosts and empty decimal ports are accepted; numbers are not range-checked. Output arguments may be changed on failure.

<a id="method-check-serialized-origin"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_serialized_origin

```cpp
static constexpr bool check_serialized_origin(std::string_view sv) noexcept;
```

Checks `scheme://host[:port]` without a path. Uses the permissive host/port rules, including empty hosts and ports.

<a id="method-check-uri-authority"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_authority

```cpp
static constexpr bool check_uri_authority(std::string_view sv) noexcept;
```

Checks an optional userinfo prefix followed by host/port. It does not resolve the host or decode escapes.

<a id="method-check-uri-reference"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check_uri_reference

```cpp
static constexpr bool check_uri_reference(std::string_view sv,
                                            bool allow_fragment) noexcept;
```

Checks a complete absolute or relative URI reference. Empty text is accepted. `allow_fragment` controls whether a fragment is allowed; escapes are checked but not decoded.

<a id="method-split-authority-host-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> split_authority_host_port

```cpp
static constexpr bool split_authority_host_port(
      std::string_view sv, std::string_view& decoded_uri_host,
      std::size_t& colon_found_at) noexcept;
```

Splits host text from its port separator, respecting bracketed literals. For bracketed input, `colon_found_at` is the position after `]`, even if no colon follows; otherwise it is the first colon or `npos`. Returns false only for an unclosed leading bracket. It does not validate the resulting host or port.

<a id="method-try-to-deserialize-as-authority-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_authority_form

```cpp
static constexpr deserialization_status try_to_deserialize_as_authority_form(
      std::string_view sv, std::string_view& uri_host, std::string_view& port,
      host_type& type, std::size_t& bytes_used) noexcept;
```

Parses a host, required colon, and a decimal port prefix; the port may be empty and is not range-checked. A missing colon or closing bracket requests more bytes. Success reports a consumed prefix, not a complete request target; output arguments may change before failure.

<a id="method-try-to-deserialize-as-asterisk-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_asterisk_form

```cpp
static constexpr deserialization_status try_to_deserialize_as_asterisk_form(
      std::string_view sv, std::size_t& bytes_used) noexcept;
```

Consumes one leading `*`. Empty input requests more bytes; a different first byte is invalid. Success does not require the rest of the input to be empty.

<a id="method-try-to-deserialize-as-origin-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_origin_form

```cpp
static constexpr deserialization_status try_to_deserialize_as_origin_form(
      std::string_view sv, std::string_view& consumed_path,
      std::string_view& consumed_query, std::size_t& bytes_used) noexcept;
```

Parses a leading absolute path and optional query as borrowed views, reporting the consumed prefix. It stops at the first delimiter outside that grammar; the caller checks the request-line separator.

<a id="method-default-port-for-scheme"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> default_port_for_scheme

```cpp
static constexpr std::string_view default_port_for_scheme(
      std::string_view scheme) noexcept;
```

Returns `80` for ASCII-case-insensitive `http`, `443` for `https`, or an empty view for other schemes.

<a id="method-ports-equivalent"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ports_equivalent

```cpp
static constexpr bool ports_equivalent(std::string_view scheme,
                                         std::string_view a,
                                         std::string_view b) noexcept;
```

Compares port strings exactly, replacing empty values with the default for HTTP/HTTPS. It does not normalize leading zeroes or parse numbers.

<a id="method-try-to-deserialize-as-absolute-form"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_absolute_form

```cpp
static constexpr deserialization_status try_to_deserialize_as_absolute_form(
      std::string_view sv, std::string_view& consumed_path,
      std::string_view& consumed_query, bool& has_authority,
      std::string_view& uri_host, std::string_view& port, host_type& type,
      std::string_view& scheme, std::size_t& bytes_used) noexcept;
```

Parses a scheme, optional authority, path, and query prefix into borrowed views. It resets outputs at entry, may update them on failure, and reports consumed bytes on success. It does not percent-decode components or validate the request-line delimiter.

<a id="method-try-to-deserialize-as-absolute-path"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_absolute_path

```cpp
static constexpr deserialization_status try_to_deserialize_as_absolute_path(
      std::string_view sv, std::string_view& consumed_path,
      std::size_t& bytes_used) noexcept;
```

Requires a leading slash and consumes path characters and valid percent escapes. An incomplete escape requests more bytes. Success may stop before the end of input; it returns a borrowed prefix and consumed count.

<a id="method-try-to-deserialize-as-query"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> try_to_deserialize_as_query

```cpp
static constexpr deserialization_status try_to_deserialize_as_query(
      std::string_view sv, std::string_view& consumed_query,
      std::size_t& bytes_used) noexcept;
```

Consumes query characters and valid percent escapes, allowing an empty query. An incomplete escape requests more bytes. Success may stop before the end of input; it returns a borrowed prefix and consumed count.

<a id="method-iequals"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> iequals

```cpp
static constexpr bool iequals(std::string_view a, std::string_view b);
```

Compares equal-length strings after folding ASCII `A-Z` only. Other bytes are compared unchanged.

<a id="method-split-query-parameters"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> split_query_parameters

```cpp
static constexpr std::size_t split_query_parameters(
      std::string_view query, std::span<std::string_view> out_keys,
      std::span<std::string_view> out_values) noexcept;
```

Splits raw text on `&`, skips empty pairs, and splits each pair at its first `=`. Stops when `out_keys` is full and returns the number written. `out_values.size()` must be at least `out_keys.size()`. Views borrow the input; no percent decoding or `+` conversion occurs.
