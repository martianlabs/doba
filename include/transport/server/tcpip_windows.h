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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_transport_server_tcpip_windows_h
#define martianlabs_doba_transport_server_tcpip_windows_h

#include <mswsock.h>

#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>

#include "protocol/deserialization.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip [windowsTM]                                           ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the WindowsTM server transport.              |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   BFsz - buffer size for I/O operations.                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, std::size_t BFsz>
class tcpip {
  // +=========================================================================+
  // | [>] CONSTANTS                                               ( private ) |
  // +=========================================================================+
  static constexpr DWORD kAcceptAddressBytes =
      static_cast<DWORD>(sizeof(sockaddr_storage) + 16);
  // Maximum number of in-flight (assigned but not-yet-sent) responses that a
  // single connection may hold out-of-order before they are drained in
  // request-arrival order. Bounds the per-connection reorder buffer and caps
  // the pipelining/async-concurrency depth we buffer; if exceeded the
  // connection is defensively closed instead of growing without bound (see
  // context::deposit_and_drain). Kept as an internal constant for now; exposing
  // it as a configurable property is a planned future evolution.
  static constexpr std::uint64_t kReorderWindow = 256;
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct context {
    // [types]
    // Outcome of depositing a response into the reorder buffer: either it was
    // accepted (and drained as far as the send cursor allows) or the reorder
    // window was exceeded and the connection must be defensively closed.
    enum class deposit_result : std::uint8_t { kOk, kOverflow };
    // [constructors/destructors]
    context(SOCKET in_socket) : socket{in_socket} {}
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    // [operators]
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    ~context() = default;
    // [push_pending_response]
    // PRECONDITION: sending_mutex MUST already be held by the caller (through a
    // sending_guard). This helper does NOT lock — locking here would deadlock,
    // as every caller (enqueue<-send_error, deposit_and_drain) already holds
    // it.
    INLINE void push_pending_response(std::unique_ptr<std::string> buffer) {
      assert_sending_held();
      if (!buffer || buffer->empty()) return;
      responses.push(std::move(buffer));
    }
    // [drain_pending_responses]
    // PRECONDITION: sending_mutex MUST already be held by the caller (through a
    // sending_guard). This helper does NOT lock (the caller, dequeue<-arm_next_
    // send_operation, already holds it; locking here would deadlock).
    // Coalesces EVERY currently-queued response into a single buffer,
    // preserving strict FIFO (arrival) order. This is what restores send
    // batching: instead of one WSASend per response, a single WSASend carries
    // all responses that are ready at this moment. Returns nullptr when the
    // queue is empty. The common single-response case is moved out without any
    // copy; only when two or more responses are pending do we concatenate them
    // into one buffer.
    INLINE std::unique_ptr<std::string> drain_pending_responses() {
      assert_sending_held();
      if (responses.empty()) return nullptr;
      std::unique_ptr<std::string> merged = std::move(responses.front());
      responses.pop();
      // Fast path: a single response was queued, hand it over untouched.
      if (responses.empty()) return merged;
      // Slow path: several responses are ready; concatenate them in FIFO order
      // so they travel in one send, exactly like the pre-ordering batching did.
      while (!responses.empty()) {
        std::unique_ptr<std::string> next = std::move(responses.front());
        responses.pop();
        if (next && !next->empty()) merged->append(*next);
      }
      return merged;
    }
    // [assign_sequence]
    // Stamps the caller with the next arrival-order sequence number. Invoked
    // from the single-in-flight receive loop, in strict request order.
    INLINE std::uint64_t assign_sequence() {
      sending_guard guard(*this);
      return next_seq_to_assign++;
    }
    // [deposit_and_drain]
    // Places 'payload' (already-serialized response bytes) at its sequence slot
    // and, starting from the send cursor, drains every contiguous ready slot
    // onto the outgoing 'responses' queue in request-arrival order. This is the
    // single place that enforces global response ordering across sync/async
    // completions. Returns kOverflow when the sequence would alias a still
    // in-flight slot (reorder window exceeded) so the caller can close the
    // connection instead of buffering unbounded or overwriting a pending slot.
    INLINE deposit_result
    deposit_and_drain(std::uint64_t seq, std::unique_ptr<std::string> payload) {
      sending_guard guard(*this);
      if (seq >= next_seq_to_send + kReorderWindow) {
        return deposit_result::kOverflow;
      }
      reorder[seq % kReorderWindow] = std::move(payload);
      // Cascade-drain contiguous ready slots from the send cursor. A slot holds
      // a non-null unique_ptr once its response has been deposited (the handler
      // contract guarantees exactly one deposit per assigned sequence). If that
      // deposited payload is empty, it still advances the cursor but is not
      // pushed, so ordering is preserved without emitting empty data. A null
      // slot means the response for that sequence has not arrived yet and stops
      // the drain (head-of-line), which is exactly the ordering guarantee.
      while (reorder[next_seq_to_send % kReorderWindow] != nullptr) {
        std::unique_ptr<std::string> ready =
            std::move(reorder[next_seq_to_send % kReorderWindow]);
        if (ready && !ready->empty()) responses.push(std::move(ready));
        next_seq_to_send++;
      }
      return deposit_result::kOk;
    }
    // [general] attributes
    // socket is atomic because it is read (WSARecv/WSASend) on send/receive
    // paths while it may be concurrently closed and reset to INVALID_SOCKET by
    // mark_context_for_closing running on another IOCP worker. Making it a
    // std::atomic<SOCKET> turns those concurrent accesses into well-defined
    // atomic loads/stores (no C++ data race) without needing to hold
    // sending_mutex around every syscall.
    std::atomic<SOCKET> socket{INVALID_SOCKET};
    std::atomic<bool> closing{false};
    // [responses] attributes
    std::queue<std::unique_ptr<std::string>> responses;
    // [ordering] attributes (all guarded by sending_mutex)
    // next_seq_to_assign: monotonic counter; every request parsed off this
    // connection is stamped with the next value (in strict arrival order, from
    // the single-in-flight receive loop) so its response can be re-sequenced.
    // next_seq_to_send: cursor of the next sequence allowed onto 'responses';
    // responses completed out of order wait in 'reorder' until it is their
    // turn, then drain in cascade. A slot holding nullptr means "not ready".
    std::uint64_t next_seq_to_assign{0};
    std::uint64_t next_seq_to_send{0};
    std::array<std::unique_ptr<std::string>, kReorderWindow> reorder{};
    // [sending] attributes
    bool close_after_sending{false};
    std::mutex sending_mutex;
    bool sending{false};
#ifndef NDEBUG
    // Debug-only owner tracking to enforce the "sending_mutex held" contract of
    // the response-queue helpers
    // (push_pending_response/drain_pending_responses) and the ordering helpers.
    // Set while a sending_guard holds the lock; used by assert_sending_held().
    // Has zero effect in release builds.
    std::atomic<std::thread::id> sending_owner_dbg{std::thread::id{}};
#endif
    // RAII lock over sending_mutex. Always acquire the mutex through this guard
    // (instead of a raw std::lock_guard) so the debug owner is recorded and the
    // "held" contract of the queue/ordering helpers is checkable. In release
    // builds it is exactly a std::lock_guard with no extra cost.
    struct sending_guard {
      explicit sending_guard(context& c) : ctx_(c), lock_(c.sending_mutex) {
#ifndef NDEBUG
        ctx_.sending_owner_dbg.store(std::this_thread::get_id());
#endif
      }
      ~sending_guard() {
#ifndef NDEBUG
        ctx_.sending_owner_dbg.store(std::thread::id{});
#endif
      }
      sending_guard(const sending_guard&) = delete;
      sending_guard& operator=(const sending_guard&) = delete;

     private:
      context& ctx_;
      std::lock_guard<std::mutex> lock_;
    };
    // Asserts (debug-only) that the calling thread currently holds
    // sending_mutex through a sending_guard. Documents and enforces the
    // precondition of the response-queue and ordering helpers below.
    INLINE void assert_sending_held() const {
#ifndef NDEBUG
      assert(sending_owner_dbg.load() == std::this_thread::get_id() &&
             "sending_mutex must be held (via sending_guard) by the caller");
#endif
    }
  };
  enum class io_type : uint8_t { kAccept, kSend, kReceive, kStop };
  struct overlapped_base : OVERLAPPED {
    overlapped_base(io_type in_type, std::shared_ptr<context> in_ctx = nullptr)
        : OVERLAPPED{}, type{in_type}, ctx{in_ctx} {}
    INLINE io_type get_type() const { return type; }
    INLINE std::shared_ptr<context> get_context() const { return ctx; }

   private:
    const io_type type;
    std::shared_ptr<context> ctx;
  };
  struct overlapped_accept : overlapped_base {
    overlapped_accept(SOCKET in_socket)
        : overlapped_base(io_type::kAccept), socket{in_socket} {}
    SOCKET socket{INVALID_SOCKET};
    CHAR addresses[(kAcceptAddressBytes * 2)]{0};
  };
  struct overlapped_receive : overlapped_base {
    overlapped_receive(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kReceive, in_ctx) {}
    WSABUF wsa;
    CHAR buffer[BFsz];
  };
  struct overlapped_send : overlapped_base {
    overlapped_send(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kSend, in_ctx) {}
    WSABUF wsa;
    std::unique_ptr<std::string> buffer;
  };
  struct overlapped_stop : overlapped_base {
    overlapped_stop() : overlapped_base(io_type::kStop) {}
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
    // If the i/o completion port is valid then we are already started!
    if (io_h_ != nullptr) return;
    // Let's setup all the required resources..
    auto workers = setup_listener(port);
    setup_workers(workers);
    setup_accept_pipeline(std::max<std::size_t>(2, workers));
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    // If the i/o completion port is not valid then we are already stopped!
    if (io_h_ == nullptr) return;
    // Let's close the listening socket and post stop messages to all workers!
    closesocket(accept_socket_);
    accept_socket_ = INVALID_SOCKET;
    for (std::size_t i = 0; i < workers_.size(); i++) {
      if (!PostQueuedCompletionStatus(io_h_, 0, 0, new overlapped_stop())) {
        /*
        pepe
        */

        /*
        pepe fin
        */
      }
    }
    workers_.clear();
    accept_ex_ = nullptr;
    accept_depth_ = 0;
    if (!CloseHandle(io_h_)) {
      /*
      pepe
      */

      /*
      pepe fin
      */
    }
    io_h_ = nullptr;
  }

 private:
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
  // +=========================================================================+
  std::size_t setup_listener(const char port[]) {
    // Let's use our help class to create a cpu pinning plan!
    std::size_t workers = std::thread::hardware_concurrency();
    // Let's create our main i/o completion port!
    HANDLE ioh = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, workers);
    if (ioh == NULL) {
      // ((error)) -> while setting up the i/o completion port!
      throw std::runtime_error("I/O Completion Port could not be created!");
    }
    // Let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> Could not create socket!
      CloseHandle(ioh);
      throw std::runtime_error("Socket could not be created!");
    }
    int port_num = std::stoi(port);
    if (port_num < 1 || port_num > 65535) {
      // ((error)) -> Could not set socket opt reuse-address!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    sockaddr_in addr = {0};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    if (bind(sock, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
      // ((error)) -> Could not bind socket!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Could not bind socket!");
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
      // ((error)) -> Could not listen socket!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Could not listen on socket!");
    }
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptex_guid,
                 sizeof(acceptex_guid), &accept_ex_, sizeof(accept_ex_), &bytes,
                 nullptr, nullptr) == SOCKET_ERROR ||
        accept_ex_ == nullptr) {
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("AcceptEx entry point could not be loaded!");
    }
    if (!CreateIoCompletionPort((HANDLE)sock, ioh, 0, 0)) {
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error(
          "Listener socket could not be associated to IOCP!");
    }
    accept_socket_ = sock;
    io_h_ = ioh;
    return workers;
  }
  // +=========================================================================+
  // | [>] setup_accept_pipeline                                   ( private ) |
  // +=========================================================================+
  void setup_accept_pipeline(std::size_t accepts_in_flight) {
    accept_depth_ = accepts_in_flight;
    for (std::size_t i = 0; i < accept_depth_; i++) {
      if (!post_accept()) {
        throw std::runtime_error("AcceptEx pipeline could not be armed!");
      }
    }
  }
  // +=========================================================================+
  // | [>] setup_workers                                           ( private ) |
  // +=========================================================================+
  void setup_workers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; i++) {
      workers_.emplace_back(std::jthread([this]() {
        bool stopping = false;
        while (!stopping) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD n = 0;  // bytes transfered..
          DWORD tout = INFINITE;
          BOOL st = GetQueuedCompletionStatus(io_h_, &n, &key, &lpo, tout);
          DWORD err = st ? ERROR_SUCCESS : GetLastError();
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (st == TRUE) {
            switch (ovb->get_type()) {
              case io_type::kAccept:
                handle_accept(reinterpret_cast<overlapped_accept*>(ovb));
                break;
              case io_type::kReceive:
                handle_receive(reinterpret_cast<overlapped_receive*>(ovb), n);
                break;
              case io_type::kSend:
                handle_send(reinterpret_cast<overlapped_send*>(ovb), n);
                break;
              case io_type::kStop:
                handle_stop(stopping);
                break;
            }
          } else {
            handle_error(ovb);
          }
          handle_overlapped(ovb);
        }
      }));
    }
  }
  // +=========================================================================+
  // | [>] handle_accept                                           ( private ) |
  // +=========================================================================+
  INLINE void handle_accept(overlapped_accept* ova) {
    if (!post_accept()) return;
    // Let's create a new context for this accepted socket!
    std::shared_ptr<context> ctx = std::make_shared<context>(ova->socket);
    // Let's set the accepted socket to be associated with the listening socket!
    int result = setsockopt(ova->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                            reinterpret_cast<const char*>(&accept_socket_),
                            sizeof(accept_socket_));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      return;
    }
    // Let's set the accepted socket to be non-blocking!
    ULONG i_mode_flag = 1;
    result = ioctlsocket(ova->socket, FIONBIO, &i_mode_flag);
    if (result != NO_ERROR) {
      closesocket(ova->socket);
      return;
    }
    // Let's set the accepted socket to be no-delay!
    int ndf = 1;
    result = setsockopt(ova->socket, IPPROTO_TCP, TCP_NODELAY,
                        reinterpret_cast<const char*>(&ndf), sizeof(ndf));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      return;
    }
    // Let's associate the accepted socket to the i/o completion port!
    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(ctx.get());
    if (!CreateIoCompletionPort((HANDLE)ova->socket, io_h_, key, 0)) {
      closesocket(ova->socket);
      return;
    }
    // Let's arm next receive operation!
    arm_next_receive_operation(ctx);
    // Let's call user's callback to notify for new connection!
    try {
      on_connection();
    } catch (const std::exception& ex) {
      mark_context_for_closing(ctx);
      return;
    } catch (...) {
      mark_context_for_closing(ctx);
      return;
    }
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  INLINE void handle_receive(overlapped_receive* ovr, DWORD bytes_received) {
    // If the associated context is in 'closing' status then the operation is
    // discarded and no actions are performed!
    if (ovr->get_context()->closing.load()) return;
    // Any of the following situations will trigger a disconnection!
    if (!bytes_received) {
      mark_context_for_closing(ovr->get_context());
      return;
    }
    // Now, we'll try to decode as many requests as possible..
    DWORD oin = 0;
    bool close_channel = false;
    // Per-frame receive-batch gate (NOT shared context state): true while THIS
    // handle_receive invocation is still decoding its batch of pipelined
    // requests off one WSARecv. Sync route callbacks fire inline below, see it
    // as 'true' and only DEPOSIT their responses (deferring the send) so the
    // whole batch is flushed as one coalesced WSASend after the loop. Because
    // this state is local to the frame (a fresh shared_ptr per call), it can
    // never be observed by an unrelated concurrent handle_receive of a later
    // batch, which is exactly the TOCTOU hazard a context-wide flag had. Each
    // callback captures this shared_ptr by value; the atomic<bool> outlives the
    // frame while any deferred async callback still references it. The store to
    // false happens right before the final flush below.
    auto batch = std::make_shared<std::atomic<bool>>(true);
    while (oin < bytes_received) {
      std::shared_ptr<RSty> res = std::make_shared<RSty>();
      std::size_t space_left_in = bytes_received - oin;
      protocol::deserialization_result<RQty> result =
          RQty::deserialize(std::string_view(&ovr->buffer[oin], space_left_in));
      if (result.code == protocol::deserialization_status::kMoreBytesNeeded) {
        // In case of 'failed' operation, bytes can NOT be greater than zero!
        if (result.bytes_used > 0) {
          // In this case we'll mark this context as 'closing'..
          try {
            on_bad_request("Inconsistent deserialization status!", res);
            send_error_and_mark_context_for_closing(ovr->get_context(), res);
          } catch (const std::exception&) {
            mark_context_for_closing(ovr->get_context());
          } catch (...) {
            mark_context_for_closing(ovr->get_context());
          }
          return;
        }
        break;
      }
      if (result.code == protocol::deserialization_status::kInvalidSource) {
        // In this case we'll mark this context as 'closing'..
        try {
          on_bad_request("Invalid source deserialization content!", res);
          send_error_and_mark_context_for_closing(ovr->get_context(), res);
        } catch (const std::exception&) {
          mark_context_for_closing(ovr->get_context());
        } catch (...) {
          mark_context_for_closing(ovr->get_context());
        }
        return;
      }
      // In case of 'succeeded' operation, consumed bytes can NOT be zero!
      // Also, used bytes in deserialization must be within valid memory range!
      if (result.bytes_used == 0 || result.bytes_used > (space_left_in)) {
        // In this case we'll mark this context as 'closing'..
        try {
          on_bad_request("Inconsistent deserialization status!", res);
          send_error_and_mark_context_for_closing(ovr->get_context(), res);
        } catch (const std::exception&) {
          mark_context_for_closing(ovr->get_context());
        } catch (...) {
          mark_context_for_closing(ovr->get_context());
        }
        return;
      }
      // In case of 'succeeded' operation, returned request cannot be NULL!
      if (result.request == nullptr) {
        // In this case we'll mark this context as 'closing'..
        try {
          on_bad_request("Inconsistent deserialization status!", res);
          send_error_and_mark_context_for_closing(ovr->get_context(), res);
        } catch (const std::exception&) {
          mark_context_for_closing(ovr->get_context());
        } catch (...) {
          mark_context_for_closing(ovr->get_context());
        }
        return;
      }
      // Let's call user handler!
      try {
        std::shared_ptr<context> ctx = ovr->get_context();
        // Stamp this request with its arrival-order sequence number BEFORE the
        // handler runs. The completion callback (which may fire inline for sync
        // routes or much later on a worker thread for async routes) captures
        // 'seq' and 'ctx' by value only, so it never aliases this frame's local
        // state. deposit_and_drain re-sequences the response into arrival order
        // regardless of when/where the callback executes.
        std::uint64_t seq = ctx->assign_sequence();
        on_request(result.request, res, [this, ctx, seq, batch](auto res) {
          // If the associated context is in 'closing' status then
          // the operation is discarded and no actions are performed!
          if (ctx->closing.load()) return;
          // Deposit this response at its sequence slot and drain any
          // contiguous ready responses onto the outgoing queue,
          // preserving arrival order.
          std::unique_ptr<std::string> payload =
              std::make_unique<std::string>(res->serialize());
          if (ctx->deposit_and_drain(seq, std::move(payload)) ==
              context::deposit_result::kOverflow) {
            // Reorder window exceeded: too many responses are
            // outstanding out of order. Defensively close instead of
            // buffering without bound.
            mark_context_for_closing(ctx);
            return;
          }
          // Batching: while THIS frame's receive loop is still
          // decoding its batch (batch == true, captured
          // per-frame), sync callbacks (which run inline here) only
          // deposit; the single arm_next_send_operation at the end
          // of the batch flushes everything coalesced into one
          // WSASend. Async callbacks run after the frame has stored
          // batch = false, so they must arm the send
          // themselves. batch is local to this handle_receive
          // call, so an unrelated concurrent handle_receive of a
          // later batch can never flip our decision (no cross-frame
          // TOCTOU).
          if (!batch->load()) arm_next_send_operation(ctx);
        });
      } catch (const std::exception& ex) {
        mark_context_for_closing(ovr->get_context());
        return;
      } catch (...) {
        mark_context_for_closing(ovr->get_context());
        return;
      }
      oin += result.bytes_used;
      // The protocol tells us, through the neutral channel_intent, what to do
      // with the channel after this message. We never inspect protocol types
      // here: we only act on this closed, universal vocabulary.
      if (result.channel == protocol::channel_intent::kClose) {
        // Close once the pending response has been flushed, and stop serving
        // any further pipelined requests on this channel.
        typename context::sending_guard snd_lock(*ovr->get_context());
        ovr->get_context()->close_after_sending = true;
        close_channel = true;
        break;
      }
      // kUpgrade is reserved for a later phase (e.g. WebSocket hand-off); for
      // now it behaves like kKeep and the channel keeps being served.
    }
    // Let's arm next receive operation only when the channel is to be kept;
    // if the protocol asked to close, we must not keep reading on it (the
    // socket is closed once the pending response has been flushed).
    if (!close_channel) {
      arm_next_receive_operation(ovr->get_context());
    }
    // Close this frame's receive-batch window: from now on any (async) callback
    // that runs will arm the send itself. Storing false BEFORE the flush below
    // guarantees no async response can get stranded: if it deposited during the
    // batch it is flushed here; if it runs after this point it observes
    // batch == false and arms its own send (idempotent via the 'sending'
    // flag). Because batch is per-frame, this store is invisible to any
    // other concurrent handle_receive.
    batch->store(false);
    // Flush the batch: this single arm_next_send_operation coalesces every sync
    // response deposited above into one WSASend (restoring send batching). Any
    // async route will flush its own response via arm_next_send_operation.
    arm_next_send_operation(ovr->get_context());
  }
  // +=========================================================================+
  // | [>] handle_send                                             ( private ) |
  // +=========================================================================+
  INLINE void handle_send(overlapped_send* ovs, DWORD bytes) {
    if (!arm_pending_send_operation(ovs, bytes)) {
      arm_next_send_operation(ovs->get_context());
    }
  }
  // +=========================================================================+
  // | [>] handle_stop                                             ( private ) |
  // +=========================================================================+
  INLINE void handle_stop(bool& stopping) { stopping = true; }
  // +=========================================================================+
  // | [>] handle_error                                            ( private ) |
  // +=========================================================================+
  INLINE void handle_error(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept:
        break;
      case io_type::kReceive:
      case io_type::kSend:
        mark_context_for_closing(ovb->get_context());
        break;
      case io_type::kStop:
        break;
    }
  }
  // +=========================================================================+
  // | [>] handle_overlapped                                       ( private ) |
  // +=========================================================================+
  INLINE void handle_overlapped(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept:
        delete reinterpret_cast<overlapped_accept*>(ovb);
        break;
      case io_type::kReceive:
        delete reinterpret_cast<overlapped_receive*>(ovb);
        break;
      case io_type::kSend:
        delete reinterpret_cast<overlapped_send*>(ovb);
        break;
      case io_type::kStop:
        delete reinterpret_cast<overlapped_stop*>(ovb);
        break;
    }
  }
  // +=========================================================================+
  // | [>] post_accept                                             ( private ) |
  // +=========================================================================+
  INLINE bool post_accept() {
    SOCKET soc = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED);
    if (soc == INVALID_SOCKET) return false;
    overlapped_accept* ova = new overlapped_accept(soc);
    DWORD received = 0;
    BOOL accepted =
        accept_ex_(accept_socket_, ova->socket, ova->addresses, 0,
                   kAcceptAddressBytes, kAcceptAddressBytes, &received, ova);
    if (accepted == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
      delete ova;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] send_error_and_mark_context_for_closing                 ( private ) |
  // +=========================================================================+
  INLINE void send_error_and_mark_context_for_closing(
      std::shared_ptr<context> ctx, std::shared_ptr<RSty> response) {
    if (response) {
      typename context::sending_guard sending_lock(*ctx);
      std::unique_ptr<std::string> out = std::make_unique<std::string>();
      ctx->close_after_sending = true;
      out->append(response->serialize());
      // Terminal (protocol-error) path: this response is pushed straight onto
      // the outgoing queue, intentionally bypassing the sequence/reorder window
      // (deposit_and_drain). The offending request was never sequence-stamped,
      // and close_after_sending stops any further reads; once flushed the
      // socket is closed, so any still-in-flight out-of-order responses are
      // discarded when the context transitions to 'closing'. This is by design
      // for a fatal connection error, not an ordering violation.
      enqueue(ctx, std::move(out));
    }
    arm_next_send_operation(ctx);
  }
  // +=========================================================================+
  // | [>] mark_context_for_closing                                ( private ) |
  // +=========================================================================+
  INLINE void mark_context_for_closing(std::shared_ptr<context> ctx) {
    // Let's prepare this context for closing!
    bool expected = false;
    if (ctx->closing.compare_exchange_strong(expected, true)) {
      // Atomically take the socket handle and reset it to INVALID_SOCKET so a
      // concurrent send/receive that loads it either sees the still-valid
      // handle (its syscall will then fail once we closesocket, handled via the
      // idempotent closing CAS) or already sees INVALID_SOCKET and bails out.
      // exchange guarantees a single closesocket even under concurrent close
      // attempts (only the caller that observes a valid handle closes it).
      SOCKET s = ctx->socket.exchange(INVALID_SOCKET);
      if (s != INVALID_SOCKET) closesocket(s);
      // Let's call user's callback to notify for disconnection!
      try {
        on_disconnection();
      } catch (const std::exception& ex) {
        // [to-do] -> add support for this!
      } catch (...) {
        // [to-do] -> add support for this!
      }
    }
  }
  // +=========================================================================+
  // | [>] arm_pending_send_operation                              ( private ) |
  // +=========================================================================+
  INLINE bool arm_pending_send_operation(overlapped_send* ovs, DWORD bytes) {
    // If the associated context is in 'closing' status then the operation is
    // not dispatched and no actions are performed!
    if (ovs->get_context()->closing.load()) return false;
    typename context::sending_guard sending_lock(*ovs->get_context());
    ovs->buffer->erase(0, bytes);
    ovs->get_context()->sending = false;
    if (ovs->buffer->empty()) return false;
    // Let's arm next send operation!
    if (!send(ovs->get_context(), std::move(ovs->buffer))) {
      mark_context_for_closing(ovs->get_context());
      ovs->get_context()->sending = false;
      return false;
    }
    ovs->get_context()->sending = true;
    return true;
  }
  // +=========================================================================+
  // | [>] arm_next_send_operation                                 ( private ) |
  // +=========================================================================+
  INLINE void arm_next_send_operation(std::shared_ptr<context> ctx) {
    // If the associated context is in 'closing' status then the operation is
    // not dispatched and no actions are performed!
    if (ctx->closing.load()) return;
    // If the associated context is already sending then the operation is not
    // dispatched and no actions are performed!
    typename context::sending_guard sending_lock(*ctx);
    if (ctx->sending) return;
    std::unique_ptr<std::string> buffer = dequeue(ctx);
    if (!buffer) {
      // The outgoing queue is drained. Only close now if a close was requested
      // AND there is nothing still in flight: every assigned sequence must have
      // been drained (next_seq_to_send == next_seq_to_assign). Otherwise an
      // async response for an earlier request is still pending; closing here
      // would drop it. That pending response's own deposit_and_drain will
      // re-invoke arm_next_send_operation, which will retry this close once the
      // queue empties again with the cursor caught up.
      if (ctx->close_after_sending &&
          ctx->next_seq_to_send == ctx->next_seq_to_assign) {
        mark_context_for_closing(ctx);
      }
      return;
    }
    // Let's arm next send operation!
    if (!send(ctx, std::move(buffer))) {
      mark_context_for_closing(ctx);
      ctx->sending = false;
      return;
    }
    ctx->sending = true;
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                              ( private ) |
  // +=========================================================================+
  INLINE void arm_next_receive_operation(std::shared_ptr<context> ctx) {
    // If the associated context is in 'closing' status then the operation is
    if (ctx->closing.load()) return;
    // Let's arm next receive operation!
    if (!receive(ctx)) mark_context_for_closing(ctx);
  }
  // +=========================================================================+
  // | [>] enqueue                                                 ( private ) |
  // +=========================================================================+
  INLINE void enqueue(std::shared_ptr<context> ctx,
                      std::unique_ptr<std::string> bf) {
    ctx->push_pending_response(std::move(bf));
  }
  // +=========================================================================+
  // | [>] dequeue                                                 ( private ) |
  // +=========================================================================+
  INLINE std::unique_ptr<std::string> dequeue(std::shared_ptr<context> ctx) {
    // Coalesce all responses ready right now into a single send. This is the
    // core of send batching: one WSASend carries every response currently
    // queued, in arrival order, instead of one syscall per response.
    return ctx->drain_pending_responses();
  }
  // +=========================================================================+
  // | [>] receive                                                 ( private ) |
  // +=========================================================================+
  INLINE bool receive(std::shared_ptr<context> ctx) {
    // Load the socket atomically: a concurrent mark_context_for_closing may be
    // closing it. If it is already gone, don't post a receive on
    // INVALID_SOCKET.
    SOCKET s = ctx->socket.load();
    if (s == INVALID_SOCKET) return false;
    DWORD f = 0, r = 0;
    overlapped_receive* ovr = new overlapped_receive(ctx);
    std::memset(&ovr->wsa, 0, sizeof(WSABUF));
    std::memset(static_cast<OVERLAPPED*>(ovr), 0, sizeof(OVERLAPPED));
    ovr->wsa.buf = ovr->buffer;
    ovr->wsa.len = BFsz;
    int res = WSARecv(s, &ovr->wsa, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] send                                                    ( private ) |
  // +=========================================================================+
  INLINE bool send(std::shared_ptr<context> ctx,
                   std::unique_ptr<std::string> buffer) {
    // Load the socket atomically: a concurrent mark_context_for_closing may be
    // closing it. If it is already gone, don't post a send on INVALID_SOCKET
    // (buffer is released as this returns, callers treat false as a close).
    SOCKET s = ctx->socket.load();
    if (s == INVALID_SOCKET) return false;
    DWORD f = 0, snt = 0;
    overlapped_send* ovs = new overlapped_send(ctx);
    std::memset(&ovs->wsa, 0, sizeof(WSABUF));
    std::memset(static_cast<OVERLAPPED*>(ovs), 0, sizeof(OVERLAPPED));
    ovs->buffer = std::move(buffer);
    ovs->wsa.buf = ovs->buffer->data();
    ovs->wsa.len = ovs->buffer->size();
    int res = WSASend(s, &ovs->wsa, 1, &snt, f, ovs, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  HANDLE io_h_ = nullptr;
  SOCKET accept_socket_ = INVALID_SOCKET;
  LPFN_ACCEPTEX accept_ex_ = nullptr;
  std::size_t accept_depth_ = 0;
  std::vector<std::jthread> workers_;
};
}  // namespace martianlabs::doba::transport::server

#endif
