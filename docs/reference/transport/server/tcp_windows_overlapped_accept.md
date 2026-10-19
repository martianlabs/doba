<a id="tcp-windows-overlapped-accept"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/title.svg" alt="tcp_windows_overlapped_accept">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/title-dark.svg" alt="tcp_windows_overlapped_accept">
</a>

**Header:** [include/transport/server/tcp_windows_overlapped_accept.h](../../../../include/transport/server/tcp_windows_overlapped_accept.h)

Windows overlapped accept operation.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[kAcceptAddressBytes](#constant-kacceptaddressbytes) ·
[overlapped_accept](#type-overlapped-accept)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::overlapped_accept

[socket](#field-socket) ·
[addresses](#field-addresses)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="constant-kacceptaddressbytes"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kAcceptAddressBytes

**CONSTANT** · *transport::server*

```cpp
static constexpr DWORD kAcceptAddressBytes =
    static_cast<DWORD>(sizeof(sockaddr_storage) + 16);
```

Defines the accept address bytes constant.

**Value:** `static_cast<DWORD>(sizeof(sockaddr_storage) + 16)`

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:37](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L37)

<a id="type-overlapped-accept"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> overlapped_accept

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct overlapped_accept : overlapped_base { ... };
```

Windows overlapped accept operation.

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:47](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L47)

<a id="field-socket"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> socket

**FIELD** · *transport::server::overlapped_accept*

```cpp
SOCKET socket{INVALID_SOCKET};
```

Stores the socket value.

**Value:** `{INVALID_SOCKET}`

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:56](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L56)

<a id="field-addresses"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> addresses

**FIELD** · *transport::server::overlapped_accept*

```cpp
CHAR addresses[(kAcceptAddressBytes * 2)]{0};
```

Stores the addresses value.

**Value:** `{0}`

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:57](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L57)
