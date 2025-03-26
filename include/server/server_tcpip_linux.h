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

#include <climits>
#include <mutex>
#include <optional>
#include <queue>
#include <system_error>
#include <thread>
#include <vector>

#ifndef martianlabs_doba_server_serverilinux_h
#define martianlabs_doba_server_serverlinux_h

namespace martianlabs::doba {

/*
// =============================================================================
// Server                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the server transport implementation (linux).
// -----------------------------------------------------------------------------
// Template parameters:
//    DEty - server transport decoder type being used
// =============================================================================
template <typename DEty>
class Server : public ServerInterface<int, DEty> {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  Server() = default;
  Server(const Server&) = delete;
  Server(Server&&) noexcept = delete;
  ~Server() { Stop(); };
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  Server& operator=(const Server&) = delete;
  Server& operator=(Server&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  // Starts current transport activity using the provided (json) configuration.
  void Start(const structured::json::Object& configuration,
             const typename DEty::RequestHandler& request_handler) override {
    // let's retrieve all needed parameters for this transport..
    std::string port = configuration[ServerConstants::kPortKey];
    unsigned int workers = configuration[ServerConstants::kNumberOfWorkersKey];
    // let's assign the user-specified request handler function..
    request_handler_ = request_handler;
    // let's setup all the required resources..
    setupListener(port, workers);
    setupWorkers(workers);
    // let's start incoming connections loop!
    unsigned int next = 0;
    sockaddr client_address;
    socklen_t address_len = sizeof(client_address);
    epoll_event events[kEpollMaxEvents_];
    while (keep_running_) {
      int n_fds =
          epoll_wait(epoll_fd_, events, kEpollMaxEvents_, kEpollTimeout_);
      if (n_fds < 0) {
        // ((Error)) -> trying to read epoll events!
        break;
      }
      for (auto i = 0; i < n_fds; i++) {
        Context* context = (Context*)events[i].data.ptr;
        if (events[i].events & EPOLLIN) {
          int fd = accept(context->fd, &client_address, &address_len);
          if (fd < 0) {
            // ((Error)) -> trying to accept a new connection!
            continue;
          }
          auto flags = fcntl(fd, F_GETFL, 0);
          if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            // ((Error)) -> trying to set socket mode to non-blocking!
            shutdown(fd, SHUT_RDWR);
            close(fd);
            continue;
          }
          if (addToEpoll(workers_[next++].first, fd) == nullptr) {
            // ((Error)) -> trying to add socket to epoll handlers list!
            shutdown(fd, SHUT_RDWR);
            close(fd);
          }
          if (next == workers) next = 0;
        }
      }
    }
    close(accept_socket_);
    close(epoll_fd_);
    epoll_fd_ = INVALID_SOCKET;
    accept_socket_ = INVALID_SOCKET;
    for (auto& worker : workers_) {
      if (worker.second->joinable()) {
        worker.second->join();
      }
    }
    delete accept_ctx_;
  }
  // Stops current transport activity.
  void Stop(void) override { keep_running_ = false; }
  // Tries to send the specified buffer through the transport connection.
  bool Send(const SOCKET& handler, const base::AutoBuffer& buffer) override {
    const char* buf = nullptr;
    std::size_t len = 0;
    if (buffer.GetInternalBuffer(buf, len)) {
      std::size_t off = 0;
      while (off < len) {
        std::size_t cur = std::min<std::size_t>(INT_MAX, len - off);
        auto res = send(handler, &buf[off], (int)cur, MSG_NOSIGNAL);
        if (res == SOCKET_ERROR) {
          if (errno != EAGAIN && errno != EWOULDBLOCK) {
            // ((Error)) -> while trying to send information to socket!
            return false;
          }
        } else {
          off += res;
        }
      }
    } else {
      char* buf = new char[base::AutoBuffer::kChunkSize];
      while (auto len = buffer.ReadSome(buf, base::AutoBuffer::kChunkSize)) {
        std::size_t offset = 0;
        while (offset < len) {
          std::size_t amount = std::min<std::size_t>(INT_MAX, len - offset);
          auto res = send(handler, &buf[offset], (int)amount, MSG_NOSIGNAL);
          if (res == SOCKET_ERROR) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
              // ((Error)) -> while trying to send information to socket!
              delete[] buf;
              return false;
            }
          } else {
            offset += res;
          }
        }
      }
      delete[] buf;
    }
    return true;
  }

 private:
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  struct Context {
    SOCKET fd;
    std::shared_ptr<DEty> decoder;
    Context(const int& in = INVALID_SOCKET) {
      decoder = std::make_shared<DEty>();
      fd = in;
    }
    ~Context() = default;
  };
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  // Sets-up listener socket resources.
  void setupListener(const std::string& port,
                     const unsigned int& number_of_workers) {
    int accept_fd, epoll_fd;
    sockaddr_in addr = {0};
    memset(&addr, 0, sizeof(addr));
    if ((accept_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      // ((Error)) -> could not create socket!
      throw std::runtime_error("could not create listening socket!");
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    uint16_t port_number = (uint16_t)atoi(port.c_str());
    addr.sin_port = htons(port_number);
    if (bind(accept_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      close(accept_fd);
      throw std::runtime_error("could not bind to listening socket!");
    }
    if (listen(accept_fd, SOMAXCONN) < 0) {
      close(accept_fd);
      throw std::runtime_error("could not bind to listening socket!");
    }
    auto flags = fcntl(accept_fd, F_GETFL, 0);
    if (fcntl(accept_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
      close(accept_fd);
      throw std::runtime_error("could not set socket to non-blocking!");
    }
    if ((epoll_fd = epoll_create1(0)) < 0) {
      throw std::runtime_error("could not create epoll handler!");
    }
    if ((accept_ctx_ = addToEpoll(epoll_fd, accept_fd)) != nullptr) {
      epoll_fd_ = epoll_fd;
      accept_socket_ = accept_fd;
    } else {
      throw std::runtime_error("could add accept-socket to epoll handler!");
    }
  }
  // Sets-up all needed workers within this transport.
  void setupWorkers(const int& number_of_workers) {
    for (int i = 0; i < number_of_workers; i++) {
      int efd = epoll_create1(0);
      if (efd < 0) {
        // ((Error)) -> trying to create epoll handler!
        throw std::runtime_error(
            "could not create private (per-thread) epoll handler!");
      }
      workers_.push_back(std::make_pair(
          efd, std::make_shared<std::thread>([efd, this]() {
            epoll_event ev[kEpollMaxEvents_];
            char buffer[base::AutoBuffer::kChunkSize];
            while (keep_running_) {
              int n_fds = epoll_wait(efd, ev, kEpollMaxEvents_, kEpollTimeout_);
              if (n_fds < 0) {
                // ((Error)) -> trying to complete wait: shutting down?
                break;
              }
              for (auto i = 0; i < n_fds; i++) {
                Context* ctx = (Context*)ev[i].data.ptr;
                if (ev[i].events & EPOLLIN) {
                  auto bytes_returned = read(ctx->fd, buffer, base::AutoBuffer::kChunkSize);
                  if (bytes_returned < 0 && errno != EAGAIN &&
                      errno != EWOULDBLOCK) {
                    removeFromEpollAndClose(efd, ctx);
                    continue;
                  }
                  ctx->decoder->Add(buffer, bytes_returned);
                  ctx->decoder->Decode(
                      request_handler_,
                      [this, ctx, efd]() { removeFromEpollAndClose(efd, ctx); },
                      [this, ctx, efd](const base::AutoBuffer& buffer) {
                        if (!Send(ctx->fd, buffer)) {
                          removeFromEpollAndClose(efd, ctx);
                        }
                      });
                }
                if (ev[i].events & EPOLLHUP || ev[i].events & EPOLLRDHUP ||
                    ev[i].events & EPOLLERR) {
                  removeFromEpollAndClose(efd, ctx);
                }
              }
            }
          })));
    }
  }
  // Adds the specifed socket to the epoll handlers list.
  Context* addToEpoll(const int& efd, const int& fd) {
    epoll_event event;
    Context* ctx = new Context(fd);
    event.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
    event.data.ptr = ctx;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) < 0) {
      // ((Error)) -> trying to register within the internal handlers list!
      delete ctx;
      ctx = nullptr;
    }
    return ctx;
  }
  // Removes the associated context from epoll (closing resources).
  void removeFromEpollAndClose(const int& efd, const Context* ctx) {
    if (epoll_ctl(efd, EPOLL_CTL_DEL, ctx->fd, NULL) < 0) {
      // ((Error)) -> trying to de-register from internal handlers list!
    }
    shutdown(ctx->fd, SHUT_RDWR);
    close(ctx->fd);
    delete ctx;
  }
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr int kEpollTimeout_ = 1000;
  static constexpr int kEpollMaxEvents_ = 256;
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  int epoll_fd_ = INVALID_SOCKET;
  int accept_socket_ = INVALID_SOCKET;
  Context* accept_ctx_ = nullptr;
  bool keep_running_ = true;
  std::vector<std::pair<int, std::shared_ptr<std::thread>>> workers_;
  typename DEty::RequestHandler request_handler_;
};
*/

} // namespace martianlabs::doba

#endif
