<a id="tcp-windows-overlapped-accept-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_accept/index.svg" alt="Index for tcp_windows_overlapped_accept.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp_windows_overlapped_accept.h](../../../../include/transport/server/tcp_windows_overlapped_accept.h)

Windows overlapped accept operation.

**transport::server**

- [kAcceptAddressBytes](#constant-kacceptaddressbytes) · *constant*
- [overlapped_accept](#type-overlapped-accept) · *type*

**transport::server::overlapped_accept**

- [overlapped_accept](#constructor-overlapped-accept) · *constructor*
- [socket](#field-socket) · *field*
- [addresses](#field-addresses) · *field*

<a id="constant-kacceptaddressbytes"></a>

## kAcceptAddressBytes

🟠 **CONSTANT** · *transport::server*

```cpp
static constexpr DWORD kAcceptAddressBytes =
    static_cast<DWORD>(sizeof(sockaddr_storage) + 16);
```

Defines the accept address bytes constant.

**Value:** `static_cast<DWORD>(sizeof(sockaddr_storage) + 16)`

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:37](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L37)

<a id="type-overlapped-accept"></a>

## overlapped_accept

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct overlapped_accept : overlapped_base { ... };
```

Windows overlapped accept operation.

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:47](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L47)

<a id="constructor-overlapped-accept"></a>

## overlapped_accept

🟠 **CONSTRUCTOR** · *transport::server::overlapped_accept*

```cpp
overlapped_accept(SOCKET in_socket);
```

Constructs a overlapped_accept value.

**Parameters:**

- **in_socket:** Initial value supplied to the constructor. Declaration: `SOCKET in_socket`.

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:51](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L51)

<a id="field-socket"></a>

## socket

🟠 **FIELD** · *transport::server::overlapped_accept*

```cpp
SOCKET socket{INVALID_SOCKET};
```

Stores the socket value.

**Value:** `{INVALID_SOCKET}`

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:56](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L56)

<a id="field-addresses"></a>

## addresses

🟠 **FIELD** · *transport::server::overlapped_accept*

```cpp
CHAR addresses[(kAcceptAddressBytes * 2)]{0};
```

Stores the addresses value.

**Value:** `{0}`

**Source:** [include/transport/server/tcp_windows_overlapped_accept.h:57](../../../../include/transport/server/tcp_windows_overlapped_accept.h#L57)
