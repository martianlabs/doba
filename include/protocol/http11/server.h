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

#ifndef martianlabs_doba_protocol_http11_server_h
#define martianlabs_doba_protocol_http11_server_h

#include <ranges>
#include <string_view>

#include "transport/server/tcpip.h"
#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/request.h"
#include "protocol/http11/response.h"
#include "protocol/http11/router.h"
#include "protocol/http11/headers.h"
#include "protocol/http11/decoder.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// server                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 server implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    TRty - transport being used (tcp/ip by default).
// =============================================================================
template <template <typename, typename, typename> class TRty =
              transport::server::tcpip>
class server {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  server(const char port[]) {
    transport_.set_port(port);
    transport_.set_number_of_workers(kDefaultNumberOfWorkers);
    transport_.set_on_connection([](socket_type id) {});
    transport_.set_on_disconnection([](socket_type id) {});
    transport_.set_on_bytes_received(
        [](socket_type id, unsigned long bytes) {});
    transport_.set_on_bytes_sent([](socket_type id, unsigned long bytes) {});
    transport_.set_on_request_ok(
        [this](const request& req,
               auto new_response) -> std::shared_ptr<response> {
          std::shared_ptr<response> res = new_response();
          if (!process_headers(req, *res)) {
            res->bad_request_400().add_header(headers::kContentLength, 0);
            return res;
          }
          switch (req.get_target()) {
            case target::kOriginForm:
            case target::kAbsoluteForm: {
              auto h = router_.match(req.get_method(), req.get_absolute_path());
              if (!h) {
                res->not_found_404().add_header(headers::kContentLength, 0);
                return res;
              }
              h->operator()(req, *res);
              // we're doing this to remove any hop-by-hop added element..
              for (auto const& hop_by_hop : res->get_hop_by_hop_headers()) {
                res->remove_header(hop_by_hop);
              }
            } break;
            case target::kAuthorityForm:
            case target::kAsteriskForm:
              // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
              // [to-do] -> add support for this!
              // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
              break;
            default:
              res->bad_request_400().add_header(headers::kContentLength, 0);
              break;
          }
          return res;
        });
    transport_.set_on_request_error(
        [this](auto new_response) -> std::shared_ptr<response> {
          std::shared_ptr<response> response = new_response();
          response->bad_request_400().add_header(headers::kContentLength, 0);
          return response;
        });
    setup_headers_functions();
  }
  server(const server&) = delete;
  server(server&&) noexcept = delete;
  ~server() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  server& operator=(const server&) = delete;
  server& operator=(server&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void start() { transport_.start(); }
  void stop() { transport_.stop(); }
  server& add_route(method method, std::string_view route, router::handler fn) {
    router_.add(method, route, fn);
    return *this;
  }

 private:
  // ___________________________________________________________________________
  // USINGs                                                          ( private )
  //
  using on_header_check_delegate =
      std::function<bool(std::string_view, const request&, response&)>;
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr uint8_t kDefaultNumberOfWorkers = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setup_headers_functions() {
    // +-----------------------------------------------------------------------+
    // | ESSENTIAL HEADERS (MANDATORY)                                         |
    // +-----------------------------------------------------------------------+
    // | [ ] Host                                                              |
    // | [x] Content-Length                                                (*) |
    // | [x] Connection                                                        |
    // | [ ] Date                                                              |
    // | [ ] Transfer-Encoding                                                 |
    // | [ ] TE                                                                |
    // +-----------------------------------------------------------------------+
    // | STRONGLY RECOMMENDED HEADERS                                          |
    // +-----------------------------------------------------------------------+
    // | [ ] Content-Type                                                      |
    // | [ ] Accept                                                            |
    // | [ ] Allow                                                             |
    // | [ ] Server                                                            |
    // | [ ] User-Agent                                                        |
    // | [ ] Expect                                                            |
    // | [ ] Upgrade                                                           |
    // +-----------------------------------------------------------------------+
    // | OPTIONAL / ADVANCED HEADERS                                           |
    // +-----------------------------------------------------------------------+
    // | [ ] Range                                                             |
    // | [ ] If-Modified-Since                                                 |
    // | [ ] Cache-Control                                                     |
    // | [ ] ETag                                                              |
    // | [ ] Location                                                          |
    // | [ ] Access-Control-*                                                  |
    // | [ ] Trailer                                                           |
    // | [ ] Vary                                                              |
    // +-----------------------------------------------------------------------+
    // +                                           (*) implemented by decoder. |
    // +-----------------------------------------------------------------------+

    // +-----------------------------------------------------------------------+
    // |                                                        [ connection ] |
    // +---------------------+-------------------------------------------------+
    // | Field               | Definition                                      |
    // +---------------------+-------------------------------------------------+
    // | Connection          | 1#connection-option                             |
    // | connection-option   | token                                           |
    // +---------------------+-------------------------------------------------+
    headers_fns_[headers::kConnection] =
        [this](std::string_view v, const request& req, response& res) -> bool {
      for (auto token : v | std::views::split(constants::character::kComma)) {
        std::string_view value(&*token.begin(), std::ranges::distance(token));
        value = helpers::ows_ltrim(helpers::ows_rtrim(value));
        if (!helpers::is_token(value)) return false;
        res.add_hop_by_hop_header(value);
      }
      return true;
    };
    // +-----------------------------------------------------------------------+
    // |                                                              [ host ] |
    // +----------------+------------------------------------------------------+
    // | Field          | Definition                                           |
    // +----------------+------------------------------------------------------+
    // | Host           | uri-host [ ":" port ]                                |
    // | uri-host       | IP-literal / IPv4address / reg-name                  |
    // | port           | *DIGIT                                               |
    // +----------------+------------------------------------------------------+
    // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                  |
    // +----------------+------------------------------------------------------+
    // | IPv6address    | 6( h16 ":" ) ls32                                    |
    // |                | / "::" 5( h16 ":" ) ls32                             |
    // |                | / [ h16 ] "::" 4( h16 ":" ) ls32                     |
    // |                | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32       |
    // |                | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32       |
    // |                | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32            |
    // |                | / [ *4( h16 ":" ) h16 ] "::" ls32                    |
    // |                | / [ *5( h16 ":" ) h16 ] "::" h16                     |
    // |                | / [ *6( h16 ":" ) h16 ] "::"                         |
    // +----------------+------------------------------------------------------+
    // | IPvFuture      | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" ) |
    // +----------------+------------------------------------------------------+
    // | ls32           | ( h16 ":" h16 ) / IPv4address                        |
    // | h16            | 1*4HEXDIG                                            |
    // +----------------+------------------------------------------------------+
    // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet  |
    // | dec-octet      | DIGIT                    ; 0-9                       |
    // |                | / %x31-39 DIGIT          ; 10-99                     |
    // |                | / "1" 2DIGIT             ; 100-199                   |
    // |                | / "2" %x30-34 DIGIT      ; 200-249                   |
    // |                | / "25" %x30-35           ; 250-255                   |
    // +----------------+------------------------------------------------------+
    // | reg-name       | *( unreserved / pct-encoded / sub-delims )           |
    // +----------------+------------------------------------------------------+
    // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                |
    // | pct-encoded    | "%" HEXDIG HEXDIG                                    |
    // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / ","  |
    // |                | / ";" / "="                                          |
    // +----------------+------------------------------------------------------+
    headers_fns_[headers::kHost] =
        [this](std::string_view v, const request& req, response& res) -> bool {
      const std::size_t kMaxDots = 3;
      auto prechech_ipv4 = [](std::string_view sv, std::size_t& cur) -> auto {
        std::vector<std::size_t> out;
        std::size_t i = 0;
        while (i < sv.length()) {
          if (!helpers::is_digit(sv[i])) {
            if (sv[i] == constants::character::kDot) {
              out.push_back(i);
            } else {
              break;
            }
          }
          i++;
        }
        if (out.size() == kMaxDots) {
          bool seems_ok = true;
          std::size_t next_to_third_dot = out[kMaxDots - 1] + 1;
          while (next_to_third_dot < sv.length()) {
            if (!helpers::is_digit(sv[next_to_third_dot])) {
              break;
            }
            next_to_third_dot++;
          }
          if (seems_ok) cur = next_to_third_dot;
        }
        return out;
      };
      auto check_ipv4 = [](std::string_view sv, std::size_t cursor,
                           const std::vector<std::size_t>& dots) -> bool {
        auto chk = [](std::string_view segment) -> bool {
          int segment_value = 0;
          auto [ptr, error_code] = std::from_chars(
              segment.data(), segment.data() + segment.size(), segment_value);
          return (error_code == std::errc() &&
                  (segment_value >= 0 && segment_value <= 255));
        };
        std::size_t sz = dots[0];
        if (chk(std::string_view(sv.data(), sz))) {
          if ((sz = dots[1] - dots[0]) > 0) {
            if (chk(std::string_view(&sv.data()[dots[0] + 1], --sz))) {
              if ((sz = dots[2] - dots[1]) > 0) {
                if (chk(std::string_view(&sv.data()[dots[1] + 1], --sz))) {
                  if ((sz = cursor - dots[2]) > 0) {
                    if (chk(std::string_view(&sv.data()[dots[2] + 1], --sz))) {
                      return true;
                    }
                  }
                }
              }
            }
          }
        }
        return false;
      };
      enum class type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
      if (!v.length()) return true;
      type potential_host_type = type::kUnknown, host_type = type::kUnknown;
      std::size_t cursor = v.length();
      std::vector<std::size_t> dots;
      if (v[0] == constants::character::kOpenBracket) {
        // check for potential [IP-literal]..
      } else {
        // check for potential [IPv4address]..
        if ((dots = prechech_ipv4(v, cursor)).size() == kMaxDots) {
          potential_host_type = type::kIpV4Address;
        }
      }
      if (potential_host_type == type::kIpLiteral) {
        // potential [IP-literal]!
      } else if (potential_host_type == type::kIpV4Address) {
        // potential [IPv4address]!
        if (check_ipv4(v, cursor, dots)) {
          host_type = potential_host_type;
        }
      }
      if (host_type == type::kUnknown) {
        // check for potential [reg-name]..
        std::size_t i = 0;
        host_type = type::kRegName;
        while (i < v.size()) {
          if (helpers::is_unreserved(v[i]) || helpers::is_sub_delim(v[i])) {
            i++;
          } else if (v[i] == constants::character::kPercent) {
            if (i + 2 < v.size() && helpers::is_hex_digit(v[i + 1]) &&
                helpers::is_hex_digit(v[i + 2])) {
              i += 3;
            } else {
              host_type = type::kUnknown;
              break;
            }
          } else if (v[i] == constants::character::kColon) {
            break;
          }
        }
        cursor = i;
      }
      // check for [ ":" port ] part..
      if (host_type != type::kUnknown) {
        if (cursor < v.size()) {
          if (v[cursor++] == constants::character::kColon) {
            while (cursor < v.size()) {
              if (!helpers::is_digit(v[cursor++])) {
                host_type = type::kUnknown;
                break;
              }
            }
          } else {
            host_type = type::kUnknown;
          }
        }
      }
      return host_type != type::kUnknown;
    };
  }
  bool process_headers(const request& req, response& res) const {
    for (auto const& hdr : req.get_headers()) {
      if (auto itr = headers_fns_.find(hdr.first); itr != headers_fns_.end()) {
        if (!itr->second(hdr.second, req, res)) return false;
      }
    }
    return true;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::unordered_map<std::string_view, on_header_check_delegate> headers_fns_;
  TRty<request, response, decoder> transport_;
  router router_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
