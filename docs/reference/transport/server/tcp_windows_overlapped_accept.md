<a id="tcp-windows-overlapped-accept-h"></a>
<h1>
  <a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/index.svg#gh-light-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/index.svg" alt="Index for tcp_windows_overlapped_accept.h">
  </a>
  <a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/index-dark.svg" alt="Index for tcp_windows_overlapped_accept.h">
  </a>
</h1>

**Header:** [include/transport/server/tcp_windows_overlapped_accept.h](../../../../include/transport/server/tcp_windows_overlapped_accept.h)

Windows overlapped accept operation.

**transport::server**

- [kAcceptAddressBytes](#constant-kacceptaddressbytes) · *constant*
- [overlapped_accept](#type-overlapped-accept) · *type*

**transport::server::overlapped_accept**

- [socket](#field-socket) · *field*
- [addresses](#field-addresses) · *field*

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
