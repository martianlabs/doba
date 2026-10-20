#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "http_server.h"
#include "common/task.h"
#include "resilience_resources.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "wait_count.h"
#include "../../../../helpers/concurrency.h"
#ifdef DOBA_ENABLE_TLS
#include "https_server.h"
#include "tls_client.h"
#include "tls_server_policies.h"
#endif

namespace {
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using namespace std::chrono_literals;
using protocol::http::v11::request;
using protocol::http::v11::response;

void require(bool condition, const char* diagnostic) {
  if (!condition) throw std::runtime_error(diagnostic);
}

struct plain_peer {
  tcpip_client socket;
  bool connect(std::uint16_t port) { return socket.connect(port); }
  bool send(std::string_view wire) { return socket.send_all(wire); }
  auto receive(std::size_t size) { return socket.receive(size); }
  bool drained() { return socket.wait_for_close(5s); }
};
#ifdef DOBA_ENABLE_TLS
struct secure_peer {
  tls_client client;
  tcpip_client& socket = client.socket;
  bool connect(std::uint16_t port) { return socket.connect(port) && client.negotiate(); }
  bool send(std::string_view wire) { return client.send_outgoing(wire); }
  auto receive(std::size_t size) { return client.receive(size); }
  bool drained() { return client.receive_close() && socket.wait_for_close(5s); }
};
#endif

template <typename Server>
struct observed_server : Server {
  using Server::Server;
  void on_stopping(std::function<void()> callback) {
    this->set_on_stopping(std::move(callback));
  }
};

struct cycle_state {
  tests::manual_event pending;
  tests::manual_event late;
  tests::manual_event stopping;
  std::atomic<int> entered{0}, completed{0}, released{0}, live{0}, fast{0};
};

template <typename Peer>
std::string receive_body(Peer& client, std::size_t capacity) {
  std::string head;
  while (!head.ends_with("\r\n\r\n")) {
    require(head.size() < 16384, "resilience response head exceeded limit");
    auto byte = client.receive(1);
    require(byte.has_value(), "resilience response head missing");
    head += *byte;
  }
  require(head.starts_with("HTTP/1.1 200 OK\r\n"), "resilience status was not 200");
  const auto field = head.find("\r\nContent-Length: ");
  require(field != std::string::npos, "resilience response has no length");
  const auto start = field + sizeof("\r\nContent-Length: ") - 1;
  const auto end = head.find("\r\n", start);
  std::size_t length = 0;
  const auto parsed = std::from_chars(head.data() + start, head.data() + end, length);
  require(parsed.ec == std::errc{} && parsed.ptr == head.data() + end && length <= 65536,
          "resilience response length invalid");
  std::string result;
  while (result.size() < length) {
    auto part = client.receive((std::min)(capacity, length - result.size()));
    require(part.has_value() && !part->empty(), "resilience body truncated");
    result += *part;
  }
  return result;
}

int measured_cycles() {
  const auto* setting = std::getenv("DOBA_RESILIENCE_CYCLES");
  if (!setting) return 6;
  int cycles = 0;
  const std::string_view value(setting);
  const auto parsed = std::from_chars(value.data(), value.data() + value.size(), cycles);
  require(parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size() &&
          cycles >= 1 && cycles <= 100, "DOBA_RESILIENCE_CYCLES must be 1..100");
  return cycles;
}

template <typename Peer, typename Server>
void run_cycles(Server& server, std::uint16_t port, const char* transport,
                resilience_temp_directory& temporary) {
  constexpr int peers = 4;
  std::shared_ptr<cycle_state> state;
  tests::scope_exit stop_before_state_destruction([&] { server.stop(); });
  server.on_stopping([&] { state->stopping.open(); });
  auto hold = [&](const request& req, response& res) -> common::task<void> {
    auto owner = state;
    ++owner->entered;
    ++owner->live;
    tests::scope_exit released([&] { --owner->live; ++owner->released; });
    if (req.get_absolute_path() == "/late") co_await owner->late;
    else co_await owner->pending;
    std::array<std::byte, 257> buffer{};
    std::string body;
    auto* source = req.get_body_reader();
    require(source != nullptr, "suspended request lost its body reader");
    for (;;) {
      const auto read = source->read(buffer);
      require(!read.has_error && read.produced <= buffer.size(), "suspended body read failed");
      body.append(reinterpret_cast<const char*>(buffer.data()), read.produced);
      if (read.complete) break;
      require(read.produced > 0 && body.size() <= 4096, "suspended body did not progress");
    }
    res.ok_200().set_body(body + std::string(32768, 'r'));
    ++owner->completed;
  };
  server.add_route("POST", "/hold", hold);
  server.add_route("POST", "/late", hold);
  server.add_route("GET", "/fast", [&](const request&, response& res) {
    res.ok_200().set_body("fast");
    ++state->fast;
  });
  const auto count = measured_cycles();
  resilience_resources baseline;
  for (int cycle = -2; cycle < count; ++cycle) {
    test_helper::set_context(std::string(transport) + " cycle " + std::to_string(cycle));
    state = std::make_shared<cycle_state>();
    {
      std::promise<void> stopped;
      auto stop_result = stopped.get_future();
      std::jthread stopper;
      tests::scope_exit release_before_join([&] { state->pending.open(); state->late.open(); });
      std::vector<std::unique_ptr<Peer>> clients;
      std::vector<std::string> payloads;
      server.start();
      for (int index = 0; index < peers; ++index) {
        auto client = std::make_unique<Peer>();
        require(client->socket.set_receive_buffer_size(4096), "cannot constrain receive window");
        require(client->connect(port), "resilience client could not connect");
        const std::string payload = std::to_string(cycle) + ":" + std::to_string(index) +
            std::string(1024, static_cast<char>('a' + index));
        const std::string wire = std::string(index == peers - 1 ? "POST /late" : "POST /hold") +
            " HTTP/1.1\r\nHost: x\r\nContent-Length: " +
            std::to_string(payload.size()) + "\r\n\r\n" + payload.substr(0, payload.size() - 1);
        require(client->send(wire), "resilience pipeline send failed");
        clients.push_back(std::move(client));
        payloads.push_back(payload);
      }
      const auto spill_deadline = std::chrono::steady_clock::now() + 5s;
      std::size_t spills = 0;
      do {
        spills = 0;
        for ([[maybe_unused]] const auto& file : std::filesystem::directory_iterator(temporary.path))
          ++spills;
        if (spills == peers) break;
        std::this_thread::yield();
      } while (std::chrono::steady_clock::now() < spill_deadline);
      require(spills == peers, "incomplete request bodies did not create spill files");
      for (int index = 0; index < peers; ++index) {
        const auto& payload = payloads[static_cast<std::size_t>(index)];
        require(clients[static_cast<std::size_t>(index)]->send(payload.substr(payload.size() - 1) +
            "GET /fast HTTP/1.1\r\nHost: x\r\n\r\n"), "resilience body completion send failed");
      }
      require(state->pending.wait_for_waiters(peers - 1) && state->late.wait_for_waiters(1),
              "not all handlers suspended");
      require(wait_count(state->fast, peers), "successors were not queued behind suspended handlers");
      require(state->live == peers && state->completed == 0, "suspension census mismatch");
      require(temporary.empty(), "completed request bodies retained temporary names");
      clients.back()->socket.abort();
      // Only bytes submitted before stop are promised by the drain contract.
      // Keep the disconnected handler pending to test retirement independently.
      state->pending.open();
      require(state->completed == peers - 1, "live handlers did not complete before drain");
      stopper = std::jthread([&] {
        try { server.stop(); stopped.set_value(); }
        catch (...) { stopped.set_exception(std::current_exception()); }
      });
      require(state->stopping.wait_for(), "server did not enter quiesce/stop");
      require(stop_result.wait_for(0ms) == std::future_status::timeout,
              "stop returned before suspended handlers finished");
      state->late.open();
      for (int index = 0; index < peers - 1; ++index) {
        auto& client = *clients[static_cast<std::size_t>(index)];
        const auto capacity = index == 0 ? 17UL : 4096UL;
        require(receive_body(client, capacity) == payloads[static_cast<std::size_t>(index)] +
                std::string(32768, 'r'), "resilience body/order mismatch");
        require(receive_body(client, capacity) == "fast", "resilience successor/order mismatch");
        require(client.drained(), "resilience connection did not drain/close");
        client.socket.close();
      }
      require(stop_result.wait_for(5s) == std::future_status::ready, "resilience stop did not complete");
      stop_result.get();
      stopper.join();
      require(state->entered == peers && state->completed == peers && state->released == peers &&
              state->live == 0 && state->fast == peers, "resilience handler lifetime census mismatch");
    }
    require(temporary.empty(), "spill files retained after drain");
    const auto resources = measure_resilience_resources();
    if (cycle == -1) baseline = resources;
    if (cycle >= 0) {
      std::fprintf(stderr, "RESILIENCE %s cycle=%d handles=%zu baseline=%zu memory_bytes=%zu\n",
                   transport, cycle, resources.handles, baseline.handles, resources.memory_bytes);
      require(resources.handles <= baseline.handles, "process handles/descriptors grew after drain");
    }
  }
  test_helper::set_context({});
}
}  // namespace

DOBA_TEST("resilience TCP drains mixed clients and restarts without retained resources") {
  resilience_temp_directory temporary;
  std::uint16_t port = 0;
  { tcpip_client probe; port = probe.find_available_port(); }
  DOBA_EXPECT(port != 0);
  protocol::http::v11::policies http;
  http.request_body_memory_threshold = 64;
  observed_server<http_server<>> server(
      {.worker_count = 2, .ip = "127.0.0.1", .port = std::to_string(port)}, http);
  run_cycles<plain_peer>(server, port, "TCP", temporary);
}

#ifdef DOBA_ENABLE_TLS
DOBA_TEST("resilience TLS drains mixed clients and restarts without retained resources") {
  resilience_temp_directory temporary;
  std::uint16_t port = 0;
  { tcpip_client probe; port = probe.find_available_port(); }
  DOBA_EXPECT(port != 0);
  protocol::http::v11::policies http;
  http.request_body_memory_threshold = 64;
  observed_server<https_server<>> server(server_policies(port), http);
  run_cycles<secure_peer>(server, port, "TLS", temporary);
}
#endif
