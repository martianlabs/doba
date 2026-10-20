#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "fuzz_helpers.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "protocol/http/v11/decoder.h"

namespace {
namespace http = martianlabs::doba::protocol::http::v11;
using status = martianlabs::doba::protocol::deserialization_status;
using doba_fuzz::check;

struct message {
  std::vector<std::string> fields;
  std::vector<std::pair<std::string, std::string>> headers;
  std::vector<std::pair<std::string, std::string>> queries;
  std::string body;
  std::size_t boundary;
  bool operator==(const message&) const = default;
};
struct stream {
  std::vector<message> messages;
  bool rejected = false;
  bool operator==(const stream&) const = default;
};

stream decode(std::string_view wire, bool fragmented, const http::policies& policy) {
  stream result;
  std::vector<std::pair<http::request, std::size_t>> retained;
  {
    http::decoder<http::request, http::response> parser(policy);
    std::size_t pos = 0;
    std::string carry;
    while (pos < wire.size() && !result.rejected) {
      const auto count = fragmented ? doba_fuzz::fragment_size(wire, pos) : wire.size() - pos;
      std::string piece = std::move(carry);
      piece.append(wire.substr(pos, count));
      pos += count;
      std::string_view input(piece);
      while (!input.empty()) {
        const auto before = input;
        auto decoded = parser.deserialize(input);
        check(input.size() <= before.size(), "decoder consumption within input");
        if (!input.empty()) {
          check(input.data() == before.data() + before.size() - input.size(),
                "decoder remainder is input suffix");
        }
        if (decoded.code == status::kSucceeded) {
          check(decoded.request.has_value(), "successful decoder returns request");
          check(input.size() < before.size(), "successful decoder makes progress");
          decoded.request->make_owned();
          retained.emplace_back(std::move(*decoded.request), pos - input.size());
        } else if (decoded.code == status::kInvalidSource) {
          result.rejected = true;
          break;  // An invalid stream is terminal; never decode its successor.
        } else {
          check(!decoded.request.has_value(), "incomplete decoder has no request");
          carry.assign(input);
          break;
        }
      }
      // Expose dependencies on caller-owned storage before the next fragment.
      std::fill(piece.begin(), piece.end(), '\x5a');
    }
  }  // Owned requests must also survive destruction of the decoder.
  for (auto& [request, boundary] : retained) {
    message snapshot{{std::string(request.get_method()),
                      std::string(request.get_absolute_path()),
                      std::string(request.get_host()),
                      std::string(request.get_host_port()),
                      std::to_string(static_cast<int>(request.get_target())),
                      std::string(request.get_target_authority_host()),
                      std::string(request.get_target_authority_port()),
                      request.wants_connection_close() ? "close" : "keep"}, {}, {}, {}, boundary};
    for (std::size_t i = 0; i < request.get_headers_length(); ++i) {
      const auto header = request.get_header(i);
      snapshot.headers.emplace_back(header.first, header.second);
    }
    for (std::size_t i = 0; i < request.get_query_parameters_length(); ++i) {
      const auto query = request.get_query_parameter(i);
      snapshot.queries.emplace_back(query.first, query.second);
    }
    if (auto* reader = request.get_body_reader()) {
      std::array<std::byte, 97> output{};
      bool complete = false;
      for (std::size_t step = 0; step <= wire.size() + 1; ++step) {
        const auto state = reader->read(output);
        check(state.produced <= output.size(), "request body output within capacity");
        check(!state.has_error, "accepted request has readable body");
        snapshot.body.append(reinterpret_cast<const char*>(output.data()), state.produced);
        check(snapshot.body.size() <= wire.size(), "request payload bounded by wire");
        if (state.complete) { complete = true; break; }
        check(state.produced > 0, "accepted body reader makes progress");
      }
      check(complete, "accepted request body completes");
      check(snapshot.body.size() <= policy.max_request_body_size, "request payload limit");
    }
    result.messages.push_back(std::move(snapshot));
  }
  return result;
}
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  if (size > 16384) return 0;
  const std::string_view wire(reinterpret_cast<const char*>(data), size);
  http::policies policy;
  policy.max_request_body_size = 4096;
  policy.max_request_head_size = size % 2 ? 512 : 4096;
  policy.max_chunk_extension_size = size % 3 ? 16 : 1024;
  policy.max_trailer_section_size = size % 3 ? 64 : 4096;
  policy.max_query_parameters = 32;
  policy.request_body_memory_threshold = 16384;
  const auto whole = decode(wire, false, policy);
  check(whole == decode(wire, true, policy), "HTTP fragmentation equivalence");
  policy.request_body_memory_threshold = size % 2 ? 0 : 64;
  check(whole == decode(wire, true, policy), "HTTP memory/disk storage equivalence");

  // Differential rejection alone could miss a parser rejecting every input.
  // Build a valid pipeline with a known binary body and exact boundaries too.
  const auto payload = wire.substr(0, 512);
  const std::string body = size % 2 ? doba_fuzz::chunked_wire(payload) : std::string(payload);
  const std::string head = "POST /fuzz HTTP/1.1\r\nHost: x\r\n" +
      (size % 2 ? std::string("Transfer-Encoding: chunked\r\n\r\n") :
                  "Content-Length: " + std::to_string(payload.size()) + "\r\n\r\n");
  const std::string successor = "GET /next HTTP/1.1\r\nHost: x\r\n\r\n";
  const auto valid_wire = head + body + successor;
  const auto known = decode(valid_wire, true, policy);
  check(!known.rejected && known.messages.size() == 2, "valid HTTP pipeline accepted");
  check(known.messages[0].body == payload && known.messages[1].body.empty(),
        "valid HTTP payload equals supplied bytes");
  check(known.messages[0].boundary == head.size() + body.size() &&
        known.messages[1].boundary == valid_wire.size(), "valid HTTP pipeline boundaries");
  check(known.messages[0].fields[1] == "/fuzz" && known.messages[1].fields[1] == "/next",
        "valid HTTP pipeline targets");
  check(known == decode(valid_wire, false, policy), "valid HTTP fragmentation equivalence");
  return 0;
}
