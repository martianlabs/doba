//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
//
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_transport_server_tcpip_linux_h
#define martianlabs_doba_transport_server_tcpip_linux_h

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "platform.h"
#include "protocol/deserialization.h"
#include "protocol/serialization.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip [linux]                                               ( class ) |
// +---------------------------------------------------------------------------+
// | Design: shared epoll fd, EPOLLONESHOT + EPOLLET per connection.           |
// | Shared epoll means one epoll_wait syscall per worker; EPOLLONESHOT means  |
// | a single worker owns each fd at a time, no per-fd locking on recv path.   |
// |                                                                           |
// | Partial sends: unsent tail stored in send_pending_; EPOLLOUT re-arm.      |
// | All ordering, batching, and close-after-send semantics match Windows.     |
// |                                                                           |
// | Ownership: each connection has a shared_ptr<context>. The fd_token stored |
// | in epoll_event.data.ptr holds a strong shared_ptr<context> and is         |
// | the sole owner of the context while the fd is registered in epoll.        |
// | The fd_token is heap-allocated in handle_accept and deleted in            |
// | mark_context_for_closing after epoll_ctl(DEL). EPOLLONESHOT guarantees    |
// | a single worker owns each fd.                                             |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request type.                                                    |
// |   RSty - response type.                                                   |
// |   BFsz - per-connection receive buffer size.                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, std::size_t BFsz>
class tcpip {
  // +=========================================================================+
  // | [>] CONSTANTS                                               ( private ) |
  // +=========================================================================+
  static constexpr int kEpollMaxEvents = 128;
  static constexpr std::uint64_t kReorderWindow = 256;
  using serialization_result_type = protocol::serialization_result;
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct context;  // forward declaration for fd_token::spp
  // Tag struct stored in epoll_event.data.ptr for every registered fd.
  // Using data.ptr exclusively (never data.fd) avoids the union aliasing trap.
  enum class fd_kind : std::uint8_t { kAccept, kConnection };
  struct fd_token {
    fd_kind kind;
    // Strong reference to the context. The fd_token is the sole strong owner
    // of the context between handle_accept and mark_context_for_closing.
    // mark_context_for_closing calls delete on the fd_token; this is safe
    // because EPOLLONESHOT prevents a second event for the same fd appearing
    // in the same already-returned epoll_wait batch.
    std::shared_ptr<context> spp;
  };
  struct context {
    // [types]
    enum class deposit_result : std::uint8_t { kOk, kOverflow };
    // [constructors/destructors]
    explicit context(int in_fd) : fd{in_fd} {}
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    // [operators]
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    ~context() = default;
    // [push_pending_response]
    INLINE void push_pending_response(
        std::unique_ptr<serialization_result_type> response) {
      if (!response) return;
      responses.push(std::move(response));
    }
    // [drain_pending_responses]
    INLINE std::unique_ptr<serialization_result_type> drain_pending_responses() {
      if (responses.empty()) return nullptr;
      std::unique_ptr<serialization_result_type> merged =
          std::move(responses.front());
      responses.pop();
      // fast path: single response, or head has a streaming body — hand over
      // untouched.
      if (responses.empty() || merged->source) return merged;
      // slow path: coalesce all contiguous prefix-only responses into one
      // buffer so they travel in a single send() call (send batching).
      while (!responses.empty() && !responses.front()->source) {
        std::unique_ptr<serialization_result_type> next =
            std::move(responses.front());
        responses.pop();
        if (!next->prefix.empty()) merged->prefix.append(next->prefix);
      }
      return merged;
    }
    // [deposit_and_drain]
    INLINE deposit_result
    deposit_and_drain(std::uint64_t seq,
                      std::unique_ptr<serialization_result_type> payload) {
      sending_guard guard(*this);
      if (seq - next_seq_to_send >= kReorderWindow) {
        return deposit_result::kOverflow;
      }
      reorder[seq % kReorderWindow] = std::move(payload);
      while (reorder[next_seq_to_send % kReorderWindow] != nullptr) {
        std::unique_ptr<serialization_result_type> ready =
            std::move(reorder[next_seq_to_send % kReorderWindow]);
        if (ready) responses.push(std::move(ready));
        next_seq_to_send++;
      }
      return deposit_result::kOk;
    }
    // [general] attributes
    std::atomic<int> fd{-1};
    std::atomic<bool> closing{false};
    std::atomic<bool> batch_receiving{false};
    // Non-owning alias of the fd_token used by rearm helpers to rebuild
    // epoll_event.data.ptr. Written once in handle_accept, nulled atomically
    // in mark_context_for_closing. The fd_token is owned by itself (heap,
    // deleted in mark_context_for_closing after epoll_ctl(DEL)).
    std::atomic<fd_token*> epoll_token_{nullptr};
    // [receive] attributes: no lock needed; EPOLLONESHOT ensures at most one
    // worker is active per fd at a time.
    char recv_buf[BFsz];
    std::size_t recv_off{0};
    // [responses] attributes
    std::queue<std::unique_ptr<serialization_result_type>> responses;
    // [ordering] attributes
    // next_seq_to_assign: written only from the receive path; EPOLLONESHOT
    // guarantees at most one worker is active per fd at a time, so no atomic
    // or lock is needed here (matches the Windows transport).
    std::uint64_t next_seq_to_assign{0};
    std::uint64_t next_seq_to_send{0};
    std::array<std::unique_ptr<serialization_result_type>, kReorderWindow>
        reorder{};
    // [sending] attributes (all guarded by sending_mutex)
    bool close_after_sending{false};
    std::mutex sending_mutex;
    bool sending{false};
    std::unique_ptr<std::string> send_pending_;
    std::size_t send_pending_off_{0};
    std::unique_ptr<serialization_result_type> active_response_;
    // [sending] types
    struct sending_guard {
      explicit sending_guard(context& c) : lock_(c.sending_mutex) {}
      sending_guard(const sending_guard&) = delete;
      sending_guard& operator=(const sending_guard&) = delete;

     private:
      std::lock_guard<std::mutex> lock_;
    };
    // RAII guard that resets batch_receiving to false on scope exit.
    // Call release() on the normal exit path where the flag is already
    // cleared explicitly, so the destructor becomes a no-op.
    struct batch_receiving_guard {
      explicit batch_receiving_guard(context& c) : ctx_(c) {}
      batch_receiving_guard(const batch_receiving_guard&) = delete;
      batch_receiving_guard& operator=(const batch_receiving_guard&) = delete;
      ~batch_receiving_guard() {
        if (!released_) ctx_.batch_receiving.store(false);
      }
      INLINE void release() { released_ = true; }

     private:
      context& ctx_;
      bool released_{false};
    };
  };

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  tcpip() = default;
  tcpip(const tcpip&) = delete;
  tcpip(tcpip&&) noexcept = delete;
  ~tcpip() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  tcpip& operator=(const tcpip&) = delete;
  tcpip& operator=(tcpip&&) noexcept = delete;
  // +=========================================================================+
  // | [>] PROPERTIEs                                               ( public ) |
  // +=========================================================================+
  types::on_request_delegate<RQty, RSty> on_request;
  types::on_bad_request_delegate<RSty> on_bad_request;
  types::on_client_connected_delegate on_connection;
  types::on_client_disconnected_delegate on_disconnection;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start(const char port[]) {
    if (epoll_fd_ != -1) return;
    auto workers = setup_listener(port);
    setup_workers(workers);
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    if (epoll_fd_ == -1) return;
    int as = accept_socket_.exchange(-1);
    if (as != -1) {
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, as, nullptr);
      ::close(as);
    }
    // request_stop() is idempotent; workers check stop_requested() on each
    // epoll_wait timeout and exit. workers_.clear() joins them all.
    for (auto& w : workers_) w.request_stop();
    workers_.clear();
    int ef = epoll_fd_;
    epoll_fd_ = -1;
    if (ef != -1) ::close(ef);
  }

 private:
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
  // +=========================================================================+
  std::size_t setup_listener(const char port[]) {
    // Suppress SIGPIPE process-wide: broken-pipe errors surface as EPIPE
    // instead of delivering a fatal signal to the process.
    ::signal(SIGPIPE, SIG_IGN);
    // hardware_concurrency() returns 0 when the value is not computable;
    // guarantee at least one worker so the server can always process events.
    std::size_t workers = std::max(1u, std::thread::hardware_concurrency());
    int efd = ::epoll_create1(EPOLL_CLOEXEC);
    if (efd < 0) throw std::runtime_error("epoll_create1 failed!");
    int sock = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP);
    if (sock < 0) {
      ::close(efd);
      throw std::runtime_error("Listening socket could not be created!");
    }
    // Single cleanup lambda: closes both fds on any error path so the
    // caller never needs to repeat ::close(sock); ::close(efd) manually.
    auto cleanup = [&] {
      ::close(sock);
      ::close(efd);
    };
    int port_num = 0;
    try {
      port_num = std::stoi(port);
    } catch (...) {
      cleanup();
      throw std::runtime_error("Invalid port number!");
    }
    if (port_num < 1 || port_num > 65535) {
      cleanup();
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    int opt = 1;
    if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      cleanup();
      throw std::runtime_error("setsockopt SO_REUSEADDR failed!");
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<std::uint16_t>(port_num));
    if (::bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) <
        0) {
      cleanup();
      throw std::runtime_error("Could not bind listening socket!");
    }
    if (::listen(sock, SOMAXCONN) < 0) {
      cleanup();
      throw std::runtime_error("Could not listen on socket!");
    }
    {
      accept_token_.kind = fd_kind::kAccept;
      epoll_event ev{};
      ev.events = EPOLLIN | EPOLLONESHOT;
      ev.data.ptr = &accept_token_;
      if (::epoll_ctl(efd, EPOLL_CTL_ADD, sock, &ev) < 0) {
        cleanup();
        throw std::runtime_error(
            "Listening socket could not be registered with epoll!");
      }
    }
    accept_socket_.store(sock);
    epoll_fd_ = efd;
    return workers;
  }
  // +=========================================================================+
  // | [>] setup_workers                                           ( private ) |
  // +=========================================================================+
  void setup_workers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; ++i) {
      workers_.emplace_back(std::jthread([this](std::stop_token st) {
        epoll_event events[kEpollMaxEvents];
        while (!st.stop_requested()) {
          // 200 ms timeout: periodic stop-token check when epoll set is idle.
          int n = ::epoll_wait(epoll_fd_, events, kEpollMaxEvents, 200);
          if (n < 0) {
            if (errno == EINTR) continue;
            break;
          }
          for (int ei = 0; ei < n; ++ei) {
            epoll_event& ev = events[ei];
            if (ev.data.ptr == nullptr) continue;
            fd_token* tok = static_cast<fd_token*>(ev.data.ptr);
            if (tok->kind == fd_kind::kAccept) {
              handle_accept();
              rearm_accept();
              continue;
            }
            std::shared_ptr<context> ctx = tok->spp;
            if (!ctx)
              continue;  // stale event (should not occur with EPOLLONESHOT)
            if (ev.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
              mark_context_for_closing(ctx);
              continue;
            }
            if (ev.events & EPOLLIN) {
              handle_receive(ctx);
            }
            if (ev.events & EPOLLOUT) {
              handle_send_resume(ctx);
            }
          }
        }
      }));
    }
  }
  // +=========================================================================+
  // | [>] handle_accept                                           ( private ) |
  // +=========================================================================+
  void handle_accept() {
    while (true) {
      int as = accept_socket_.load();
      if (as == -1) return;
      int fd = ::accept4(as, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
      if (fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        if (errno == EINTR) continue;
        return;
      }
      int ndf = 1;
      if (::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &ndf, sizeof(ndf)) < 0) {
        ::close(fd);
        continue;
      }
      {
        std::shared_ptr<context> ctx = std::make_shared<context>(fd);
        // fd_token is heap-allocated and owns ctx (shared_ptr, strong ref).
        // It is deleted in mark_context_for_closing after epoll_ctl(DEL).
        // EPOLLONESHOT guarantees no second event for this fd can appear in
        // the same already-returned epoll_wait batch, so delete is safe there.
        fd_token* tok = new fd_token{fd_kind::kConnection, ctx};
        ctx->epoll_token_.store(tok);
        epoll_event ev{};
        ev.events =
            EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
        ev.data.ptr = tok;
        if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
          ctx->epoll_token_.store(nullptr);
          delete tok;
          ::close(fd);
          continue;
        }
        try {
          on_connection();
        } catch (...) {
          mark_context_for_closing(ctx);
        }
      }
    }
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  void handle_receive(std::shared_ptr<context> ctx) {
    if (ctx->closing.load()) return;
    // batch_receiving=true: sync callbacks deposit only; end-of-frame flush
    // coalesces. After frame exits, async callbacks arm the send themselves.
    // The guard resets the flag automatically on any early return so callers
    // never need to remember to clear it on error paths.
    ctx->batch_receiving.store(true);
    typename context::batch_receiving_guard brg(*ctx);
    while (true) {
      if (ctx->closing.load()) return;
      std::size_t free_space = BFsz - ctx->recv_off;
      if (free_space == 0) {
        mark_context_for_closing(ctx);
        return;
      }
      ssize_t n =
          ::recv(ctx->fd.load(), ctx->recv_buf + ctx->recv_off, free_space, 0);
      if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
        if (errno == EINTR) continue;
        mark_context_for_closing(ctx);
        return;
      }
      if (n == 0) {
        mark_context_for_closing(ctx);
        return;
      }
      std::size_t total = ctx->recv_off + static_cast<std::size_t>(n);
      std::size_t oin = 0;
      bool had_partial = false;
      while (oin < total) {
        std::size_t space_left_in = total - oin;
        protocol::deserialization_result<RQty> result = RQty::deserialize(
            std::string_view(&ctx->recv_buf[oin], space_left_in));
        if (result.code == protocol::deserialization_status::kMoreBytesNeeded) {
          if (result.bytes_used > 0) {
            report_error_and_close(ctx, "Inconsistent deserialization status!");
            return;
          }
          std::size_t m = total - oin;
          if (m == BFsz) {
            mark_context_for_closing(ctx);
            return;
          }
          // Slide the residual bytes to the front so the next recv() appends
          // contiguously. recv_off is set here; the outer loop must NOT reset
          // it to 0 on this iteration (had_partial guards that).
          if (oin > 0) std::memmove(ctx->recv_buf, ctx->recv_buf + oin, m);
          ctx->recv_off = m;
          had_partial = true;
          break;  // partial request: keep bytes, go back to recv
        }
        if (result.code == protocol::deserialization_status::kInvalidSource) {
          report_error_and_close(ctx,
                                 "Invalid source deserialization content!");
          return;
        }
        if (result.bytes_used == 0 || result.bytes_used > space_left_in) {
          report_error_and_close(ctx, "Inconsistent deserialization status!");
          return;
        }
        if (result.request == nullptr) {
          report_error_and_close(ctx, "Inconsistent deserialization status!");
          return;
        }
        {
          std::shared_ptr<RSty> res = std::make_shared<RSty>();
          try {
            std::uint64_t seq = ctx->next_seq_to_assign++;
            on_request(result.request, res, [this, ctx, seq](auto res) {
              if (ctx->closing.load()) return;
              std::unique_ptr<serialization_result_type> payload =
                  std::make_unique<serialization_result_type>(res->serialize());
              if (ctx->deposit_and_drain(seq, std::move(payload)) ==
                  context::deposit_result::kOverflow) {
                mark_context_for_closing(ctx);
                return;
              }
              if (!ctx->batch_receiving.load()) arm_next_send_operation(ctx);
            });
          } catch (...) {
            mark_context_for_closing(ctx);
            return;
          }
        }
        oin += result.bytes_used;
        if (result.channel == protocol::channel_intent::kClose) {
          {
            typename context::sending_guard snd_lock(*ctx);
            ctx->close_after_sending = true;
          }
          ctx->recv_off = 0;
          // brg destructor will clear batch_receiving on return.
          arm_next_send_operation(ctx);
          return;
        }
      }  // inner while (oin < total)
      // Reset the carry-over offset only when all bytes in this recv batch
      // were fully consumed. When had_partial is true, recv_off was already
      // set inside the inner loop and must not be overwritten here.
      if (!had_partial) ctx->recv_off = 0;
    }  // outer recv loop (until EAGAIN)
    // Normal exit: clear batch_receiving BEFORE re-arm so no second worker
    // enters handle_receive with the flag still true. Release the guard so
    // its destructor becomes a no-op (we clear the flag manually here to
    // control the ordering with respect to the re-arm calls below).
    brg.release();
    ctx->batch_receiving.store(false);
    rearm_recv_only(ctx);
    arm_next_send_operation(ctx);
  }
  // +=========================================================================+
  // | [>] handle_send_resume                                      ( private ) |
  // +=========================================================================+
  INLINE void handle_send_resume(std::shared_ptr<context> ctx) {
    if (ctx->closing.load()) return;
    arm_pending_send_operation(ctx);
  }
  // +=========================================================================+
  // | [>] rearm_accept                                            ( private ) |
  // +=========================================================================+
  INLINE void rearm_accept() {
    int as = accept_socket_.load();
    if (as == -1) return;
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLONESHOT;
    ev.data.ptr = &accept_token_;
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, as, &ev);
  }
  // +=========================================================================+
  // | [>] report_error_and_close                                  ( private ) |
  // +=========================================================================+
  // Calls on_bad_request with the given message, sends the response, and
  // marks the connection for closing. Centralises the try/catch boilerplate
  // shared by all protocol-error paths in handle_receive.
  void report_error_and_close(std::shared_ptr<context> ctx,
                              const char* message) {
    try {
      std::shared_ptr<RSty> res = std::make_shared<RSty>();
      on_bad_request(message, res);
      send_error_and_mark_context_for_closing(ctx, res);
    } catch (...) {
      mark_context_for_closing(ctx);
    }
  }
  // +=========================================================================+
  // | [>] send_error_and_mark_context_for_closing                 ( private ) |
  // +=========================================================================+
  void send_error_and_mark_context_for_closing(std::shared_ptr<context> ctx,
                                               std::shared_ptr<RSty> response) {
    if (response) {
      std::unique_ptr<serialization_result_type> out =
          std::make_unique<serialization_result_type>(response->serialize());
      {
        typename context::sending_guard sending_lock(*ctx);
        ctx->close_after_sending = true;
        // Bypass the reorder window: this is a fatal-error response for a
        // request that was never sequence-stamped.
        ctx->push_pending_response(std::move(out));
      }
    } else {
      // No response to send; still mark the connection for closure so it does
      // not remain open indefinitely after a protocol error is detected.
      typename context::sending_guard sending_lock(*ctx);
      ctx->close_after_sending = true;
    }
    arm_next_send_operation(ctx);
  }
  // +=========================================================================+
  // | [>] mark_context_for_closing                                ( private ) |
  // +=========================================================================+
  void mark_context_for_closing(std::shared_ptr<context> ctx) {
    bool expected = false;
    if (!ctx->closing.compare_exchange_strong(expected, true)) return;
    int fd = ctx->fd.exchange(-1);
    if (fd != -1) {
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
      // Null and delete the fd_token. Safe: EPOLLONESHOT guarantees no second
      // event for this fd can appear in the batch already returned to workers.
      // The fd_token's shared_ptr<context> kept ctx alive; after delete it no
      // longer contributes to the refcount (but the caller still holds ctx).
      fd_token* tok = ctx->epoll_token_.exchange(nullptr);
      delete tok;
      ::close(fd);
    }
    try {
      on_disconnection();
    } catch (...) {
    }
  }
  // +=========================================================================+
  // | [>] arm_pending_send_operation                              ( private ) |
  // +=========================================================================+
  void arm_pending_send_operation(std::shared_ptr<context> ctx) {
    if (ctx->closing.load()) return;
    bool need_close = false;
    {
      typename context::sending_guard sending_lock(*ctx);
      if (ctx->send_pending_) {
        const char* data = ctx->send_pending_->data() + ctx->send_pending_off_;
        std::size_t remaining =
            ctx->send_pending_->size() - ctx->send_pending_off_;
        std::size_t sent = 0;
        bool would_block = false;
        if (!do_send_raw(ctx->fd.load(), data, remaining, sent, would_block)) {
          ctx->sending = false;
          ctx->send_pending_.reset();
          ctx->send_pending_off_ = 0;
          need_close = true;
        } else {
          ctx->send_pending_off_ += sent;
          if (would_block ||
              ctx->send_pending_off_ < ctx->send_pending_->size()) {
            rearm_send_with_out(ctx);
            return;
          }
          ctx->send_pending_.reset();
          ctx->send_pending_off_ = 0;
        }
      }
      if (!need_close) {
        ctx->sending = false;
        if (!load_next_buffer(*ctx)) {
          if (ctx->active_response_ && ctx->active_response_->source &&
              ctx->active_response_->source->failed()) {
            need_close = true;
          } else {
            ctx->active_response_.reset();
          }
        }
        if (!need_close && !ctx->send_pending_) {
          std::unique_ptr<serialization_result_type> response =
              ctx->drain_pending_responses();
          if (response) {
            ctx->active_response_ = std::move(response);
            if (!load_next_buffer(*ctx)) {
              if (ctx->active_response_->source &&
                  ctx->active_response_->source->failed()) {
                need_close = true;
              } else {
                ctx->active_response_.reset();
              }
            }
          }
        }
        if (!need_close && !ctx->send_pending_) {
          if (ctx->close_after_sending &&
              ctx->next_seq_to_send == ctx->next_seq_to_assign) {
            need_close = true;
          } else {
            rearm_recv_only(ctx);
          }
        } else if (!need_close && !send_pending(ctx)) {
          need_close = true;
        } else {
          ctx->sending = true;
        }
      }
    }  // sending_lock released
    if (need_close) mark_context_for_closing(ctx);
  }
  // +=========================================================================+
  // | [>] load_next_buffer                                        ( private ) |
  // +---------------------------------------------------------------------------
  // | Materializes at most one transport-sized segment from the active        |
  // | response. Headers/inline bytes are sent first; body chunks are read     |
  // | lazily so no complete streaming body is accumulated in memory.          |
  // +=========================================================================+
  bool load_next_buffer(context& ctx) {
    if (!ctx.active_response_) return false;
    if (!ctx.active_response_->prefix.empty()) {
      ctx.send_pending_ = std::make_unique<std::string>(
          std::move(ctx.active_response_->prefix));
      ctx.send_pending_off_ = 0;
      return true;
    }
    if (!ctx.active_response_->source || ctx.active_response_->source->eof() ||
        ctx.active_response_->source->failed()) {
      return false;
    }
    ctx.send_pending_ = std::make_unique<std::string>(BFsz, '\0');
    std::size_t bytes = ctx.active_response_->source->read(std::span<std::byte>(
        reinterpret_cast<std::byte*>(ctx.send_pending_->data()), BFsz));
    ctx.send_pending_->resize(bytes);
    ctx.send_pending_off_ = 0;
    return bytes > 0;
  }
  // +=========================================================================+
  // | [>] send_pending                                            ( private ) |
  // +=========================================================================+
  bool send_pending(std::shared_ptr<context> ctx) {
    std::unique_ptr<std::string> buffer = std::move(ctx->send_pending_);
    ctx->send_pending_off_ = 0;
    return send(ctx, std::move(buffer));
  }
  // +=========================================================================+
  // | [>] arm_next_send_operation                                 ( private ) |
  // +=========================================================================+
  void arm_next_send_operation(std::shared_ptr<context> ctx) {
    if (ctx->closing.load()) return;
    bool need_close = false;
    {
      typename context::sending_guard sending_lock(*ctx);
      if (ctx->sending) return;
      while (!ctx->send_pending_ && !need_close) {
        if (!ctx->active_response_) {
          ctx->active_response_ = ctx->drain_pending_responses();
          if (!ctx->active_response_) break;
        }
        if (load_next_buffer(*ctx)) break;
        if (ctx->active_response_->source &&
            ctx->active_response_->source->failed()) {
          need_close = true;
        } else {
          ctx->active_response_.reset();
        }
      }
      if (!ctx->active_response_ && !ctx->send_pending_) {
        if (ctx->close_after_sending &&
            ctx->next_seq_to_send == ctx->next_seq_to_assign) {
          need_close = true;
        }
      } else if (!need_close && !send_pending(ctx)) {
        need_close = true;
      } else {
        ctx->sending = true;
      }
    }  // sending_lock released
    if (need_close) mark_context_for_closing(ctx);
  }
  // +=========================================================================+
  // | [>] send                                                    ( private ) |
  // +=========================================================================+
  bool send(std::shared_ptr<context> ctx, std::unique_ptr<std::string> buffer) {
    int fd = ctx->fd.load();
    if (fd == -1) return false;
    if (!buffer || buffer->empty()) return true;
    std::size_t sent = 0;
    bool would_block = false;
    if (!do_send_raw(fd, buffer->data(), buffer->size(), sent, would_block)) {
      return false;
    }
    if (would_block || sent < buffer->size()) {
      // Partial send: store the buffer and track the offset; wait for EPOLLOUT.
      // Do NOT erase() the sent prefix — that is O(N).
      // arm_pending_send_operation uses send_pending_off_ to skip the
      // already-sent bytes on resume.
      ctx->send_pending_ = std::move(buffer);
      ctx->send_pending_off_ = sent;
      rearm_send_with_out(ctx);
      return true;
    }
    // Defer the next body chunk (or response) to a fresh EPOLLOUT event rather
    // than draining a large source while this worker still owns the fd.
    rearm_send_with_out(ctx);
    return true;
  }
  // +=========================================================================+
  // | [>] do_send_raw                                             ( private ) |
  // +=========================================================================+
  bool do_send_raw(int fd, const char* data, std::size_t len, std::size_t& sent,
                   bool& would_block) {
    sent = 0;
    would_block = false;
    if (fd == -1 || !data) return false;
    if (len == 0) return true;
    ssize_t r = ::send(fd, data, len, MSG_NOSIGNAL);
    if (r < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        would_block = true;
        return true;
      }
      if (errno == EINTR) {
        would_block = true;  // retry via EPOLLOUT
        return true;
      }
      return false;
    }
    sent = static_cast<std::size_t>(r);
    return true;
  }
  // +=========================================================================+
  // | [>] rearm_recv_only                                         ( private ) |
  // +=========================================================================+
  INLINE void rearm_recv_only(const std::shared_ptr<context>& ctx) {
    int fd = ctx->fd.load();
    fd_token* tok = ctx->epoll_token_.load();
    if (fd == -1 || tok == nullptr) return;
    epoll_event ev{};
    ev.events =
        EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    ev.data.ptr = tok;
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
  }
  // +=========================================================================+
  // | [>] rearm_send_with_out                                     ( private ) |
  // +=========================================================================+
  INLINE void rearm_send_with_out(const std::shared_ptr<context>& ctx) {
    int fd = ctx->fd.load();
    fd_token* tok = ctx->epoll_token_.load();
    if (fd == -1 || tok == nullptr) return;
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT | EPOLLRDHUP |
                EPOLLHUP | EPOLLERR;
    ev.data.ptr = tok;
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  int epoll_fd_{-1};
  std::atomic<int> accept_socket_{-1};
  fd_token accept_token_{};
  std::vector<std::jthread> workers_;
};
}  // namespace martianlabs::doba::transport::server

#endif
