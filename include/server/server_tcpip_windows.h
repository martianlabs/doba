//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_server_serverwindows_h
#define martianlabs_doba_server_serverwindows_h

namespace martianlabs::doba::server {
// =============================================================================
// server_tcpip_windows                                            ( interface )
// -----------------------------------------------------------------------------
// This specification holds for the WindowsTM server implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    PRty - server processor type being used.
// =============================================================================
template <typename PRty>
class server_tcpip {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  server_tcpip() = default;
  server_tcpip(const server_tcpip&) = delete;
  server_tcpip(server_tcpip&&) noexcept = delete;
  virtual ~server_tcpip() { stop(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  server_tcpip& operator=(const server_tcpip&) = delete;
  server_tcpip& operator=(server_tcpip&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void start(const std::string& port, const uint8_t& number_of_workers) {
    if (io_port_ != INVALID_HANDLE_VALUE) {
      // ((Error)) -> server already initialized!
      throw std::logic_error("Already initialized server!");
    }
    // let's setup all the required resources..
    setupWinsock();
    setupListener(port, number_of_workers);
    setupWorkers(number_of_workers);
    // let's start incoming connections loop!
    manager_ = std::make_shared<std::thread>([this]() {
      while (keep_running_) {
        SOCKET client = WSAAccept(accept_socket_, NULL, NULL, NULL, NULL);
        if (client == INVALID_SOCKET) {
          // ((Error)) -> trying to accept a new connection: shutting down?
          break;
        }
        // set the socket i/o mode: In this case FIONBIO enables or disables the
        // blocking mode for the socket based on the numerical value of iMode.
        // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
        ULONG i_mode = 1;
        auto ioctl_socket_res = ioctlsocket(client, FIONBIO, &i_mode);
        if (ioctl_socket_res != NO_ERROR) {
          // ((Error)) -> could not change blocking mode on socket!
          // ((To-Do)) -> raise an exception?
        }
        // let's associate the accept socket with the i/o port!
        ULONG_PTR key = (ULONG_PTR) new Context(client);
        if (!CreateIoCompletionPort((HANDLE)client, io_port_, key, 0)) {
          // ((Error)) -> trying to associate socket to the i/o port!
          // ((To-Do)) -> raise an exception?
        }
        // let's notify waiting thread for the new connection!
        if (!PostQueuedCompletionStatus(io_port_, 0, key, NULL)) {
          // ((Error)) -> trying to notify waiting thread!
          // ((To-Do)) -> raise an exception?
        }
      }
      if (io_port_ != INVALID_HANDLE_VALUE) {
        for (auto& worker : workers_) {
          if (worker->joinable()) {
            worker->join();
          }
        }
        io_port_ = INVALID_HANDLE_VALUE;
        accept_socket_ = INVALID_SOCKET;
      }
    });
  }
  void stop() {
    if (io_port_ != INVALID_HANDLE_VALUE) {
      if (CloseHandle(io_port_) == FALSE) {
        throw std::logic_error("There were errors while closing i/o port!");
      }
    }
    if (accept_socket_ != INVALID_SOCKET) {
      if (closesocket(accept_socket_) == SOCKET_ERROR) {
        throw std::logic_error(
            "There were errors while closing accept socket!");
      }
    }
    keep_running_ = false;
    if (manager_->joinable()) {
      manager_->join();
    }
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kChunkSizeInBytes = 1024;
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  struct Context {
    WSAOVERLAPPED overlapped;
    WSABUF data;
    SOCKET socket;
    martianlabs::doba::buffer buf;
    Context(const SOCKET& in = INVALID_SOCKET) {
      ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
      data.buf = new CHAR[kChunkSizeInBytes];
      data.len = kChunkSizeInBytes;
      socket = in;
    }
    ~Context() { delete[] data.buf; }
  };
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setupWinsock() {
    struct WsaInitializer_ {
      WsaInitializer_() {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
          // ((Error)) -> winsock could not be initialized!
          throw std::runtime_error("could not initialize winsock!");
        }
      }
      ~WsaInitializer_() { WSACleanup(); }
      WSADATA wsa_data;
    };
    static std::shared_ptr<WsaInitializer_> wsa_initializer_ptr_ =
        std::make_shared<WsaInitializer_>();
  }
  void setupListener(const std::string& port,
                     const uint8_t& number_of_workers) {
    // let's create our main i/o completion port!
    auto io_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL,
                                          number_of_workers);
    if (io_port == nullptr) {
      // ((Error)) -> while setting up the i/o completion port!
      throw std::runtime_error("could not setup i/o completion port!");
    }
    // let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((Error)) -> could not create socket!
      CloseHandle(io_port);
      throw std::runtime_error("could not create listening socket!");
    }
    // let's associate the listening port to the i/o completion port!
    if (CreateIoCompletionPort((HANDLE)sock, io_port, 0UL, 0) == nullptr) {
      // ((Error)) -> while setting up the i/o completion port!
      CloseHandle(io_port);
      closesocket(sock);
      throw std::runtime_error("could not setup i/o completion port!");
    }
    // set the socket i/o mode: In this case FIONBIO enables or disables the
    // blocking mode for the socket based on the numerical value of iMode.
    // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
    ULONG i_mode = 0;
    auto ioctl_socket_res = ioctlsocket(sock, FIONBIO, &i_mode);
    if (ioctl_socket_res != NO_ERROR) {
      // ((Error)) -> could not change blocking mode on socket!
      CloseHandle(io_port);
      closesocket(sock);
      throw std::runtime_error("could not set listening socket i/o mode!");
    }
    sockaddr_in address = {0};
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = PF_INET;
    address.sin_port = htons(atoi(port.c_str()));
    auto bind_res = bind(sock, (const sockaddr*)&address, sizeof(address));
    if (bind_res == SOCKET_ERROR) {
      // ((Error)) -> could not bind socket!
      CloseHandle(io_port);
      closesocket(sock);
      throw std::runtime_error("could not bind to listening socket!");
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
      // ((Error)) -> could not listen socket!
      CloseHandle(io_port);
      closesocket(sock);
      throw std::runtime_error("could not bind to listening socket!");
    }
    accept_socket_ = sock;
    io_port_ = io_port;
  }
  void setupWorkers(const uint8_t& number_of_workers) {
    for (int i = 0; i < number_of_workers; i++) {
      workers_.push_back(std::make_shared<std::thread>([this]() {
        ULONG_PTR completion_key = NULL;
        LPOVERLAPPED overlapped = NULL;
        DWORD bytes_returned = 0;
        while (true) {
          BOOL status = GetQueuedCompletionStatus(io_port_, &bytes_returned,
                                                  (PULONG_PTR)&completion_key,
                                                  &overlapped, INFINITE);
          if (!status)
          {
            if (!overlapped) {
              if (GetLastError() == ERROR_INVALID_HANDLE) {
                break;
              }
            }
            continue;
          } 
          if (!completion_key) {
            continue;
          }
          Context* context = (Context*)completion_key;
          if (overlapped && !bytes_returned) {
            // connection closed! let's free the associated resources!
            closesocket(context->socket);
            delete context;
            continue;
          }
          if (bytes_returned) {
            context->buf.append(context->data.buf, bytes_returned);
            std::optional<typename PRty::req> req;
            try {
              req = PRty::req::deserialize(context->buf);
            } catch (const std::exception&) {
              // connection closed! let's free the associated resources!
              closesocket(context->socket);
              delete context;
              continue;
            }
            if (req.has_value()) {
              std::optional<typename PRty::res> res =
                  PRty::process(req.value());
              if (res.has_value()) {
                if (!send_response(context->socket,
                                   PRty::res::serialize(res.value()))) {
                  // connection closed! let's free the associated resources!
                  closesocket(context->socket);
                  delete context;
                  continue;
                }
              }
            }
          }
          DWORD recv_flags = 0;
          DWORD bytes_transmitted = 0;
          int wsarecv_result = WSARecv(
              context->socket, &context->data, 0x1, &bytes_transmitted,
              &recv_flags, (LPWSAOVERLAPPED)&context->overlapped, nullptr);
          if (wsarecv_result == SOCKET_ERROR) {
            int wsaLastError = WSAGetLastError();
            if (wsaLastError != WSA_IO_PENDING) {
              // connection closed! let's free the associated resources!
              closesocket(context->socket);
              delete context;
            }
          }
        }
      }));
    }
  }
  bool send_response(const SOCKET& socket, const buffer& data) {
    const char* const buf = (const char* const)data.data();
    std::size_t len = data.used();
    std::size_t off = 0;
    while (off < len) {
      std::size_t cur = std::min<std::size_t>(INT_MAX, len - off);
      auto res = send(socket, &buf[off], (int)cur, 0);
      if (res == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
        // ((Error)) -> while trying to send information to socket!
        return false;
      }
      off += res;
    }
    return true;
  }

  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  HANDLE io_port_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::vector<std::shared_ptr<std::thread>> workers_;
  std::shared_ptr<std::thread> manager_;
  PRty protocol_;
};
}  // namespace martianlabs::doba::server

#endif