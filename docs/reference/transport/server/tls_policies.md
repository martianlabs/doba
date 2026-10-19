<a id="tls-policies"></a>
<a href="../../../../resources/docs/reference/transport/server/tls_policies/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls_policies/title.svg" alt="tls_policies">
</a>
<a href="../../../../resources/docs/reference/transport/server/tls_policies/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls_policies/title-dark.svg" alt="tls_policies">
</a>

**Header:** [include/transport/server/tls_policies.h](../../../../include/transport/server/tls_policies.h)

Provides the tls policies transport component.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[tls_policies](#type-tls-policies)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::tls_policies

[kDefaultEncryptedReceiveBufferCapacity](#constant-kdefaultencryptedreceivebuffercapacity) ·
[kDefaultNetworkBioBufferCapacity](#constant-kdefaultnetworkbiobuffercapacity) ·
[certificate_file](#field-certificate-file) ·
[private_key_file](#field-private-key-file) ·
[encrypted_receive_buffer_capacity](#field-encrypted-receive-buffer-capacity) ·
[network_bio_buffer_capacity](#field-network-bio-buffer-capacity)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-tls-policies"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tls_policies

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct tls_policies : tcp_policies { ... };
```

Provides the tls policies transport component.

**Source:** [include/transport/server/tls_policies.h:39](../../../../include/transport/server/tls_policies.h#L39)

<a id="constant-kdefaultencryptedreceivebuffercapacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultEncryptedReceiveBufferCapacity

**CONSTANT** · *transport::server::tls_policies*

```cpp
static constexpr std::size_t kDefaultEncryptedReceiveBufferCapacity =
      17 * 1024;
```

Encrypted socket input capacity per connection, in bytes.

**Value:** `17 * 1024`

**Source:** [include/transport/server/tls_policies.h:41](../../../../include/transport/server/tls_policies.h#L41)

<a id="constant-kdefaultnetworkbiobuffercapacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultNetworkBioBufferCapacity

**CONSTANT** · *transport::server::tls_policies*

```cpp
static constexpr std::size_t kDefaultNetworkBioBufferCapacity = 17 * 1024;
```

Maximum TLS BIO buffer capacity, in bytes.

**Value:** `17 * 1024`

**Source:** [include/transport/server/tls_policies.h:44](../../../../include/transport/server/tls_policies.h#L44)

<a id="field-certificate-file"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> certificate_file

**FIELD** · *transport::server::tls_policies*

```cpp
std::string certificate_file;
```

Path to the server certificate chain in PEM format.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tls_policies.h:46](../../../../include/transport/server/tls_policies.h#L46)

<a id="field-private-key-file"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> private_key_file

**FIELD** · *transport::server::tls_policies*

```cpp
std::string private_key_file;
```

Path to the matching private key in PEM format.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tls_policies.h:48](../../../../include/transport/server/tls_policies.h#L48)

<a id="field-encrypted-receive-buffer-capacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> encrypted_receive_buffer_capacity

**FIELD** · *transport::server::tls_policies*

```cpp
std::size_t encrypted_receive_buffer_capacity =
      kDefaultEncryptedReceiveBufferCapacity;
```

Encrypted receive capacity per connection (must be positive).

**Value:** `kDefaultEncryptedReceiveBufferCapacity`

**Source:** [include/transport/server/tls_policies.h:50](../../../../include/transport/server/tls_policies.h#L50)

<a id="field-network-bio-buffer-capacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> network_bio_buffer_capacity

**FIELD** · *transport::server::tls_policies*

```cpp
std::size_t network_bio_buffer_capacity = kDefaultNetworkBioBufferCapacity;
```

Network BIO capacity per connection (must be positive).

**Value:** `kDefaultNetworkBioBufferCapacity`

**Source:** [include/transport/server/tls_policies.h:53](../../../../include/transport/server/tls_policies.h#L53)
