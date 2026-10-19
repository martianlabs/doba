<a id="parsed-types"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/parsed_types/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/parsed_types/index.svg" alt="parsed_types">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/parsed_types/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/parsed_types/index-dark.svg" alt="parsed_types">
</a>

**Header:** [include/protocol/http/v11/parsed_types.h](../../../../../include/protocol/http/v11/parsed_types.h)

The zero-copy product of a "host [ ":" port ]" value (the shape shared by the Host header and an authority-form request-target). "host" is the uri-host substring (bracketed colons of an IP-literal are kept intact); "type" classifies it; "port" is the raw *DIGIT run after the ":" (empty when no port is present). Both "host" and "port" may be empty

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[parsed_host_port](#type-parsed-host-port) ·
[parsed_token_list](#type-parsed-token-list) ·
[parsed_parameter_list](#type-parsed-parameter-list) ·
[parsed_scalar](#type-parsed-scalar) ·
[parsed_via_element](#type-parsed-via-element) ·
[parsed_via_list](#type-parsed-via-list) ·
[parsed_forwarded_pair](#type-parsed-forwarded-pair) ·
[parsed_forwarded_element](#type-parsed-forwarded-element) ·
[parsed_forwarded_list](#type-parsed-forwarded-list) ·
[parsed_host_port_list](#type-parsed-host-port-list)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_host_port

[host](#field-host) ·
[port](#field-port) ·
[type](#field-type) ·
[scheme](#field-scheme)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_token_list

[elements](#field-elements)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_parameter_list

[elements](#field-elements-2)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_scalar

[value](#field-value)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_via_element

[received_protocol](#field-received-protocol) ·
[received_by](#field-received-by) ·
[comment](#field-comment)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_via_list

[elements](#field-elements-3)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_forwarded_pair

[name](#field-name) ·
[value](#field-value-2)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_forwarded_element

[pairs](#field-pairs)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_forwarded_list

[elements](#field-elements-4)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::parsed_host_port_list

[elements](#field-elements-5)

<a id="type-parsed-host-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_host_port

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_host_port { ... };
```

The zero-copy product of a "host [ ":" port ]" value (the shape shared by the Host header and an authority-form request-target). "host" is the uri-host substring (bracketed colons of an IP-literal are kept intact); "type" classifies it; "port" is the raw *DIGIT run after the ":" (empty when no port is present). Both "host" and "port" may be empty

**Source:** [include/protocol/http/v11/parsed_types.h:53](../../../../../include/protocol/http/v11/parsed_types.h#L53)

<a id="field-host"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> host

**FIELD** · *protocol::http::v11::parsed_host_port*

```cpp
std::string_view host;
```

Stores the host value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:54](../../../../../include/protocol/http/v11/parsed_types.h#L54)

<a id="field-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> port

**FIELD** · *protocol::http::v11::parsed_host_port*

```cpp
std::string_view port;
```

Stores the port value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:55](../../../../../include/protocol/http/v11/parsed_types.h#L55)

<a id="field-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> type

**FIELD** · *protocol::http::v11::parsed_host_port*

```cpp
helpers::host_type type = helpers::host_type::kUnknown;
```

Stores the type value.

**Value:** `helpers::host_type::kUnknown`

**Source:** [include/protocol/http/v11/parsed_types.h:56](../../../../../include/protocol/http/v11/parsed_types.h#L56)

<a id="field-scheme"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> scheme

**FIELD** · *protocol::http::v11::parsed_host_port*

```cpp
std::string_view scheme;
```

Stores the scheme value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:57](../../../../../include/protocol/http/v11/parsed_types.h#L57)

<a id="type-parsed-token-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_token_list

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_token_list { ... };
```

The zero-copy product of a comma-separated token list (the shape shared by Connection, Trailer, Upgrade, and X-Forwarded-Proto). "elements" holds every non-empty list element, already OWS-trimmed and in the order they appeared. An empty list yields an empty "elements" vector.

**Source:** [include/protocol/http/v11/parsed_types.h:72](../../../../../include/protocol/http/v11/parsed_types.h#L72)

<a id="field-elements"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> elements

**FIELD** · *protocol::http::v11::parsed_token_list*

```cpp
std::vector<std::string_view> elements;
```

Stores the elements value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:73](../../../../../include/protocol/http/v11/parsed_types.h#L73)

<a id="type-parsed-parameter-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_parameter_list

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_parameter_list { ... };
```

The zero-copy product of a comma-separated list whose elements each carry an optional set of ";"-separated parameters (the shape shared by Transfer-Encoding and TE). Every element keeps its raw text, including any parameters, so the semantic layer decides which parameters to read.

**Source:** [include/protocol/http/v11/parsed_types.h:88](../../../../../include/protocol/http/v11/parsed_types.h#L88)

<a id="field-elements-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> elements

**FIELD** · *protocol::http::v11::parsed_parameter_list*

```cpp
std::vector<std::string_view> elements;
```

Stores the elements value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:89](../../../../../include/protocol/http/v11/parsed_types.h#L89)

<a id="type-parsed-scalar"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_scalar

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_scalar { ... };
```

The zero-copy product of a single scalar field value (the shape shared by Expect and, before numeric conversion, Max-Forwards). "value" is the raw field-value with no OWS around it, exactly as validated by the checker.

**Source:** [include/protocol/http/v11/parsed_types.h:101](../../../../../include/protocol/http/v11/parsed_types.h#L101)

<a id="field-value"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> value

**FIELD** · *protocol::http::v11::parsed_scalar*

```cpp
std::string_view value;
```

Stores the value value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:102](../../../../../include/protocol/http/v11/parsed_types.h#L102)

<a id="type-parsed-via-element"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_via_element

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_via_element { ... };
```

received-protocol = [ protocol-name "/" ] protocol-version received-by       = pseudonym / ( uri-host [ ":" port ] ) The zero-copy product of a single Via list element. "received_protocol" and "received_by" are the raw substrings of the two mandatory parts, and "comment" is the optional trailing comment (empty when absent).

**Source:** [include/protocol/http/v11/parsed_types.h:119](../../../../../include/protocol/http/v11/parsed_types.h#L119)

<a id="field-received-protocol"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> received_protocol

**FIELD** · *protocol::http::v11::parsed_via_element*

```cpp
std::string_view received_protocol;
```

Stores the received protocol value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:120](../../../../../include/protocol/http/v11/parsed_types.h#L120)

<a id="field-received-by"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> received_by

**FIELD** · *protocol::http::v11::parsed_via_element*

```cpp
std::string_view received_by;
```

Stores the received by value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:121](../../../../../include/protocol/http/v11/parsed_types.h#L121)

<a id="field-comment"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> comment

**FIELD** · *protocol::http::v11::parsed_via_element*

```cpp
std::string_view comment;
```

Stores the comment value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:122](../../../../../include/protocol/http/v11/parsed_types.h#L122)

<a id="type-parsed-via-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_via_list

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_via_list { ... };
```

The zero-copy product of a Via header: every list element in order.

**Source:** [include/protocol/http/v11/parsed_types.h:134](../../../../../include/protocol/http/v11/parsed_types.h#L134)

<a id="field-elements-3"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> elements

**FIELD** · *protocol::http::v11::parsed_via_list*

```cpp
std::vector<parsed_via_element> elements;
```

Stores the elements value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:135](../../../../../include/protocol/http/v11/parsed_types.h#L135)

<a id="type-parsed-forwarded-pair"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_forwarded_pair

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_forwarded_pair { ... };
```

The zero-copy product of a single Forwarded name=value pair. "name" is the raw parameter token and "value" is the raw token or quoted-string exactly as it appeared (quotes and escapes are preserved).

**Source:** [include/protocol/http/v11/parsed_types.h:149](../../../../../include/protocol/http/v11/parsed_types.h#L149)

<a id="field-name"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> name

**FIELD** · *protocol::http::v11::parsed_forwarded_pair*

```cpp
std::string_view name;
```

Stores the name value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:150](../../../../../include/protocol/http/v11/parsed_types.h#L150)

<a id="field-value-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> value

**FIELD** · *protocol::http::v11::parsed_forwarded_pair*

```cpp
std::string_view value;
```

Stores the value value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:151](../../../../../include/protocol/http/v11/parsed_types.h#L151)

<a id="type-parsed-forwarded-element"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_forwarded_element

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_forwarded_element { ... };
```

The zero-copy product of a single Forwarded list element: its ";"- separated name=value pairs, in order.

**Source:** [include/protocol/http/v11/parsed_types.h:164](../../../../../include/protocol/http/v11/parsed_types.h#L164)

<a id="field-pairs"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> pairs

**FIELD** · *protocol::http::v11::parsed_forwarded_element*

```cpp
std::vector<parsed_forwarded_pair> pairs;
```

Stores the pairs value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:165](../../../../../include/protocol/http/v11/parsed_types.h#L165)

<a id="type-parsed-forwarded-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_forwarded_list

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_forwarded_list { ... };
```

The zero-copy product of a Forwarded header: every list element in order.

**Source:** [include/protocol/http/v11/parsed_types.h:177](../../../../../include/protocol/http/v11/parsed_types.h#L177)

<a id="field-elements-4"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> elements

**FIELD** · *protocol::http::v11::parsed_forwarded_list*

```cpp
std::vector<parsed_forwarded_element> elements;
```

Stores the elements value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:178](../../../../../include/protocol/http/v11/parsed_types.h#L178)

<a id="type-parsed-host-port-list"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> parsed_host_port_list

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct parsed_host_port_list { ... };
```

de-facto X-Forwarded-Host - #( uri-host [ ":" port ] ) The zero-copy product of a comma-separated list of "host [ ":" port ]" items (the shape of X-Forwarded-Host), each already parsed into its host/port parts.

**Source:** [include/protocol/http/v11/parsed_types.h:192](../../../../../include/protocol/http/v11/parsed_types.h#L192)

<a id="field-elements-5"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> elements

**FIELD** · *protocol::http::v11::parsed_host_port_list*

```cpp
std::vector<parsed_host_port> elements;
```

Stores the elements value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/parsed_types.h:193](../../../../../include/protocol/http/v11/parsed_types.h#L193)
