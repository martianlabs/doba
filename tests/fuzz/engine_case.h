//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
//
// Copyright 2026 martianLabs
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

#ifndef martianlabs_doba_tests_fuzz_engine_case_h
#define martianlabs_doba_tests_fuzz_engine_case_h

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "fuzz_helpers.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "protocol/http/v11/engine.h"

namespace doba_fuzz {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_case                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | A bounded independent model of ordered mixed HTTP response streams.       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class engine_case {
 public:
  // +=========================================================================+
  // | [>] run                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  static void run(std::span<const std::uint8_t> input) {
    engine_case current(input);
    current.execute();
  }

 private:
  using request = martianlabs::doba::protocol::http::v11::request;
  using response = martianlabs::doba::protocol::http::v11::response;
  using engine =
      martianlabs::doba::protocol::http::v11::engine<request, response>;
  using router = martianlabs::doba::protocol::http::router<request, response>;
  using reader = martianlabs::doba::common::reader;
  using task = martianlabs::doba::common::task<>;
  using progress = martianlabs::doba::protocol::processing_result;

  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                ( private ) |
  // +-------------------------------------------------------------------------+
  explicit engine_case(std::span<const std::uint8_t> input) : input_(input) {
    count_ = 1 + byte(1) % 12;
    mode_ = byte(0) / 16 % 3;  // Normal, ordered close, malformed request.
    valid_ = mode_ == 0 ? count_ : byte(2) % count_ + (mode_ == 1);
    expected_ = valid_ + (mode_ == 2);
    for (std::size_t i = 0; i < count_; ++i) {
      std::string body(1 + byte(i + 7) % 128, '\0');
      for (std::size_t n = 0; n < body.size(); ++n)
        body[n] = char((n * 37 + i * 17 + byte(n + i)) & 255);
      bodies_[i] = std::make_shared<const std::string>(std::move(body));
    }
    routes_.add("GET", "/s/:id", [this](const request& req, response& res,
                                       int id) {
      mark(static_cast<std::size_t>(id));
      fill(req, res, static_cast<std::size_t>(id));
    });
    routes_.add("GET", "/a/:id", [this](const request& req, response& res,
                                       std::string_view text) -> task {
      return asynchronous(req, res, text, frame_owner_);
    });
    typename engine::policies_type policy;
    policy.max_pending_requests = 1 + byte(0) % 4;
    policy.max_task_steps_per_turn = 1 + byte(1) % 4;
    engine_ = std::make_unique<engine>(policy, routes_);
    engine_->set_on_send([this](auto head, auto body, auto source) {
      if (head.empty() && body.empty() && !source) return;
      check(!held_, "engine cannot emit through an active terminal reader");
      check(close_calls_ == 0, "engine cannot emit after terminal close");
      wire_.append(head).append(body);
      held_ = std::move(source);
      check(wire_.size() <= 65536, "model output stays bounded");
    });
    engine_->set_on_close([this] { ++close_calls_; });
  }

  // +=========================================================================+
  // | [>] asynchronous                                            ( private ) |
  // +-------------------------------------------------------------------------+
  // The owner is a coroutine parameter: it survives final_suspend too.
  task asynchronous(const request& req, response& res, std::string_view text,
                    std::shared_ptr<int> retained) {
    const auto initial = req.get_header("X-Seq");
    check(!initial.first.empty(), "initial request identity");
    unsigned initial_id = 0;
    const auto first = std::from_chars(initial.second.data(),
                         initial.second.data() + initial.second.size(),
                         initial_id);
    check(first.ec == std::errc{} &&
          first.ptr == initial.second.data() + initial.second.size(),
          "initial identity");
    mark(initial_id);
    int id = -1;
    // Parameter and header views must remain valid after suspension.
    co_await martianlabs::doba::common::yield();
    const auto parsed = std::from_chars(text.data(),
                                        text.data() + text.size(), id);
    check(parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size()
          && id >= 0, "owned asynchronous parameter survives RX reuse");
    check(static_cast<unsigned>(id) == initial_id,
          "typed parameter identity survives suspension");
    co_await martianlabs::doba::common::yield();
    check(retained == frame_owner_, "managed frame keeps its owner");
    fill(req, res, static_cast<std::size_t>(id));
  }
  // +=========================================================================+
  // | [>] byte                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  unsigned byte(std::size_t index) const {
    return input_.empty() ? 0 : input_[index % input_.size()];
  }
  // +=========================================================================+
  // | [>] mark                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  void mark(std::size_t id) {
    check(id < valid_ && !invoked_[id],
          "handler starts once and never crosses a terminal frontier");
    invoked_[id] = true;
  }
  // +=========================================================================+
  // | [>] fill                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  void fill(const request& req, response& res, std::size_t id) {
    const auto header = req.get_header("X-Seq");
    check(!header.first.empty() && header.second == std::to_string(id),
          "owned request headers survive sibling and RX reuse");
    res.ok_200().add_header("X-Seq", header.second);
    if (byte(id + 3) & 2) {
      res.set_body(reader::shared(bodies_[id],
                   std::as_bytes(std::span(*bodies_[id]))),
                   bodies_[id]->size());
    } else {
      res.set_body(*bodies_[id]);
    }
  }
  // +=========================================================================+
  // | [>] drain                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void drain() {
    std::array<std::byte, 17> block{};
    while (held_) {
      const auto size = held_->read(block);
      if (!size) {
        held_.reset();
        break;
      }
      wire_.append(reinterpret_cast<const char*>(block.data()), size);
      check(wire_.size() <= 65536, "model reader output stays bounded");
    }
  }
  // +=========================================================================+
  // | [>] execute                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void execute() {
    std::string input;
    for (std::size_t id = 0; id < count_; ++id) {
      if (mode_ == 2 && id == valid_) {
        constexpr std::array<std::string_view, 4> invalid{
            "GET /bad HTTP/1.1\r\n\r\n",
            "GET /bad HTTP/1.1\r\nHost: x\r\nHost: y\r\n\r\n",
            "POST /bad HTTP/1.1\r\nHost: x\r\nContent-Length: 1\r\n"
            "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
            "GET /bad HTTP/1.1\r\nHost: x\r\nX Bad: value\r\n\r\n"};
        input += invalid[byte(3) % invalid.size()];
        continue;
      }
      input += std::string(byte(id + 3) & 1 ? "GET /a/" : "GET /s/") +
               std::to_string(id) + " HTTP/1.1\r\nHost: x\r\nX-Seq: " +
               std::to_string(id) + "\r\n";
      if (mode_ == 1 && id + 1 == valid_) input += "Connection: close\r\n";
      input += "\r\n";
    }
    for (std::size_t offset = 0; offset < input.size();) {
      const auto size = std::min(input.size() - offset,
                                 std::size_t{1} + byte(offset) % 31);
      std::string fragment = input.substr(offset, size);
      state_ = engine_->on_bytes_received(fragment);
      std::fill(fragment.begin(), fragment.end(), '\x5a');
      offset += size;
      if (byte(offset + 5) & 1)
        state_ = engine_->on_processing_turn(false);
      if (held_ && (byte(offset + 6) & 1)) {
        drain();
        state_ = engine_->on_processing_turn(true);
      }
    }
    for (unsigned step = 0; step < 1024; ++step) {
      if (held_) {
        state_ = engine_->on_processing_turn(false);
        drain();
      } else if (!state_.turn_requested && !state_.turn_after_output_drained) {
        break;
      }
      state_ = engine_->on_processing_turn(true);
    }
    check(!held_ && !state_.turn_requested && !state_.turn_at &&
          !state_.turn_after_output_drained, "bounded work reaches quiescence");
    verify();
    check((close_calls_ != 0) == (mode_ != 0), "terminal close is requested");
    if (mode_ != 0) {
      const auto bytes = wire_.size();
      engine_->on_bytes_received("GET /s/0 HTTP/1.1\r\nHost: x\r\n\r\n");
      engine_->on_processing_turn(true);
      check(wire_.size() == bytes && close_calls_ != 0,
            "terminal engine ignores later input and turns");
    }
    engine_.reset();
    check(frame_owner_.use_count() == 1, "every suspended frame is retired");
    for (std::size_t id = 0; id < count_; ++id)
      check(bodies_[id].use_count() == 1, "every reader owner is retired");
  }
  // +=========================================================================+
  // | [>] verify                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  // Parse the wire independently; never obtain expected bytes from serialize().
  void verify() const {
    std::string_view remaining(wire_);
    for (std::size_t id = 0; id < expected_; ++id) {
      const auto end = remaining.find("\r\n\r\n");
      check(end != std::string_view::npos && end <= 4096,
            "response has a bounded complete head");
      auto head = remaining.substr(0, end);
      const bool error = mode_ == 2 && id == valid_;
      check(head.starts_with(error ? "HTTP/1.1 400 " : "HTTP/1.1 200 "),
            "response status and request order match the model");
      std::size_t length = 0, lengths = 0, sequences = 0;
      auto line = head.find("\r\n");
      while (line != std::string_view::npos) {
        head.remove_prefix(line + 2);
        line = head.find("\r\n");
        const auto field = head.substr(0, line);
        const auto colon = field.find(':');
        check(colon != std::string_view::npos, "response field delimiter");
        auto name = std::string(field.substr(0, colon));
        for (auto& c : name) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        auto value = field.substr(colon + 1);
        while (!value.empty() &&
               (value.front() == ' ' || value.front() == '\t'))
          value.remove_prefix(1);
        check(name != "transfer-encoding", "model expects unambiguous framing");
        if (name == "content-length") {
          ++lengths;
          const auto parsed = std::from_chars(value.data(),
                                      value.data() + value.size(), length);
          check(parsed.ec == std::errc{} &&
                parsed.ptr == value.data() + value.size(), "response length");
        }
        if (name == "x-seq") {
          ++sequences;
          check(value == std::to_string(id), "response identity is ordered");
        }
      }
      check(lengths == 1 && length <= 1024, "one bounded response length");
      remaining.remove_prefix(end + 4);
      check(remaining.size() >= length, "response payload is complete");
      if (!error) {
        check(sequences == 1 && invoked_[id], "expected handler was executed");
        check(remaining.substr(0, length) == *bodies_[id],
              "response retains every binary payload byte");
      }
      remaining.remove_prefix(length);
    }
    check(remaining.empty(), "no duplicate or out-of-order trailing output");
  }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Caller-owned immutable control bytes.
  std::span<const std::uint8_t> input_;
  std::size_t count_{0}, valid_{0}, expected_{0};
  unsigned mode_{0}, close_calls_{0};
  std::array<std::shared_ptr<const std::string>, 12> bodies_;
  std::array<bool, 12> invoked_{};
  std::shared_ptr<int> frame_owner_{std::make_shared<int>(0)};
  std::string wire_;  // Independently checked response stream.
  std::unique_ptr<reader> held_;  // Deliberately delayed source consumption.
  progress state_;
  router routes_;
  std::unique_ptr<engine> engine_;  // Destroy before all borrowed model state.
};
}  // namespace doba_fuzz

#endif
