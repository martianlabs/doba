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

#ifndef martianlabs_doba_protocol_http11_decoder_h
#define martianlabs_doba_protocol_http11_decoder_h

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// decoder                                                             ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 server decoder implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class decoder {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  decoder() {
    buf_sz_ = request::kDefaultFullBufferMemorySize;
    bod_sz_ = request::kDefaultBodyBufferMemorySize;
    slh_sz_ = buf_sz_ - bod_sz_;
    buf_ = (char*)malloc(buf_sz_);
    cur_ = 0;
    body_processing_ = false;
    body_size_expected_ = 0;
    body_size_received_ = 0;
    method_ = method::kUnknown;
    target_ = target::kUnknown;
  }
  decoder(const decoder&) = delete;
  decoder(decoder&&) noexcept = delete;
  ~decoder() { free(buf_); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  decoder& operator=(const decoder&) = delete;
  decoder& operator=(decoder&&) noexcept = delete;
  // ___________________________________________________________________________
  // STATIC-METHODs                                                   ( public )
  //
  static void reset(std::shared_ptr<decoder> decoder) {
    if (decoder) decoder->reset();
  }
  static void reset(std::shared_ptr<request> request) {
    if (request) request->reset();
  }
  static void reset(std::shared_ptr<response> response) {
    if (response) response->reset();
  }
  static auto serialize(std::shared_ptr<response> response) {
    return response ? response->serialize() : nullptr;
  }
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  bool add(const char* ptr, std::size_t size) {
    if ((slh_sz_ - cur_) < size) {
      // ((error)): the size of the incoming buffer exceeds limits!
      return false;
    }
    memcpy(&buf_[cur_], ptr, size);
    cur_ += size;
    return true;
  }
  std::shared_ptr<request> process() {
    static const auto eoh_len = sizeof(constants::string::kEndOfHeaders) - 1;
    static const auto eol_len = sizeof(constants::string::kCrLf) - 1;
    static const char* eoh = (const char*)constants::string::kEndOfHeaders;
    static const char* eol = (const char*)constants::string::kCrLf;
    bool ready = false;
    if (!body_processing_) {
      // let's detect the [end-of-headers] position..
      if ((hdr_end_ = get(buf_, cur_, eoh, eoh_len)).has_value()) {
        // let's detect the [end-of-request-line] position..
        rln_end_ = get(buf_, hdr_end_.value(), eol, eol_len);
        if (rln_end_.has_value() || !(hdr_end_.value() - rln_end_.value())) {
          // let's check [request-line] section..
          if (check_request_line()) {
            // let's check [headers] section..
            if (check_headers()) {
              // let's setup the request to be returned!
              const auto rline_end = rln_end_.value() + eol_len;
              const auto hdrs_end = hdr_end_.value() + eoh_len;
              request_ = std::make_shared<request>();
              request_->set(buf_, rline_end, &buf_[rline_end],
                            hdrs_end - rline_end);
              request_->set_method(method_);
              request_->set_target(target_);
              request_->set_absolute_path(abs_path_);
              if (ready = !body_processing_) {
                cur_ -= hdrs_end;
              }
            } else {
              // ((error)) -> wrong content received!
            }
          } else {
            // ((error)) -> wrong content received!
          }
        } else {
          // ((error)) -> wrong content received!
        }
      }
    }
    if (body_processing_) {
      if (body_size_expected_) {
        const auto hdrs_end = hdr_end_.value() + eoh_len;
        auto bytes_available = cur_ - hdrs_end;
        const auto remaining = body_size_expected_ - body_size_received_;
        auto bbytes = bytes_available < remaining ? bytes_available : remaining;
        body_size_received_ += bbytes;
        if (ready = body_size_received_ == body_size_expected_) {
          if (request_->add_body(&buf_[hdrs_end], body_size_received_)) {
            if (bytes_available > body_size_received_) {
              auto sz_to_move = bytes_available - body_size_received_;
              memmove(buf_, &buf_[hdrs_end + body_size_expected_], sz_to_move);
            }
            cur_ -= hdrs_end + body_size_received_;
            body_processing_ = false;
          } else {
            // ((error)) -> out of system limits???
          }
        }
      } else {
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        // [to-do] -> add support for transfer-encoding!
        // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      }
    }
    return ready ? std::move(request_) : nullptr;
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kHttpVersionLen = 8;
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void reset() {
    cur_ = 0;
    body_processing_ = false;
    body_size_expected_ = 0;
    body_size_received_ = 0;
    method_ = method::kUnknown;
    target_ = target::kUnknown;
    method_ = method::kUnknown;
    target_ = target::kUnknown;
    request_.reset();
    abs_path_.clear();
    query_.clear();
    version_.clear();
    hdr_end_.reset();
    rln_end_.reset();
    mtd_end_.reset();
    pth_end_.reset();
    ver_end_.reset();
  }
  bool check_request_line() {
    std::size_t i = 0;
    // +---------+-------------------------------------------------------------+
    // | Field   | Definition                                                  |
    // +---------+-------------------------------------------------------------+
    // | method  | token                                                       |
    // | token   | 1*tchar                                                     |
    // | tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" /                   |
    // |         | "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"               |
    // |         | / DIGIT / ALPHA                                             |
    // +---------+-------------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110                 |
    // +----------------+------------------------------------------------------+
    while (i < rln_end_.value()) {
      if (buf_[i] == constants::character::kSpace) break;
      if (!helpers::is_token(buf_[i])) return false;
      buf_[i++] = buf_[i];
    }
    mtd_end_ = i++;
    std::string_view method_str((const char*)buf_, mtd_end_.value());
    switch (method_str.length()) {
      case 3:
        if (!method_str.compare(constants::method::kGet)) {
          method_ = method::kGet;
        } else if (!method_str.compare(constants::method::kPut)) {
          method_ = method::kPut;
        } else {
          return false;
        }
        break;
      case 4:
        if (!method_str.compare(constants::method::kHead)) {
          method_ = method::kHead;
        } else if (!method_str.compare(constants::method::kPost)) {
          method_ = method::kPost;
        } else {
          return false;
        }
        break;
      case 5:
        if (!method_str.compare(constants::method::kTrace)) {
          method_ = method::kTrace;
        } else {
          return false;
        }
        break;
      case 6:
        if (!method_str.compare(constants::method::kDelete)) {
          method_ = method::kDelete;
        } else {
          return false;
        }
        break;
      case 7:
        if (!method_str.compare(constants::method::kConnect)) {
          method_ = method::kConnect;
        } else if (!method_str.compare(constants::method::kOptions)) {
          method_ = method::kOptions;
        } else {
          return false;
        }
        break;
      default:
        return false;
        break;
    }
    if (i >= rln_end_.value()) return false;
    // +=======================================================================+
    // | HTTP/1.1: request-target (RFC 9112 §2.2) – Valid Forms                |
    // +=======================================================================+
    // |                                                                       |
    // | request-target = origin-form                                          |
    // |                / absolute-form                                        |
    // |                / authority-form                                       |
    // |                / asterisk-form                                        |
    // |                                                                       |
    // |-----------------------------------------------------------------------|
    // | FORM 1: origin-form (most common)                                     |
    // |    origin-form = absolute-path [ "?" query ]                          |
    // |-----------------------------------------------------------------------|
    // | FORM 2: absolute-form (used via HTTP proxy)                           |
    // |    URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]          |
    // |-----------------------------------------------------------------------|
    // | FORM 3: authority-form (used with CONNECT)                            |
    // |    authority-form = host ":" port                                     |
    // |-----------------------------------------------------------------------|
    // | FORM 4: asterisk-form (used with OPTIONS)                             |
    // |    asterisk-form = "*"                                                |
    // +=======================================================================+
    if (method_ == method::kConnect) {
      // +---------------------------------------------------------------------+
      // |                                                      authority-form |
      // +----------------+----------------------------------------------------+
      // | Rule           | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | authority-form | authority                                          |
      // |                | (RFC 9112 §2.2)                                    |
      // | authority      | [ userinfo "@" ] host [ ":" port ]                 |
      // |                | (RFC 3986 §3.2)                                    |
      // |                |  userinfo is forbidden in HTTP `authority-form`    |
      // | host           | IP-literal / IPv4address / reg-name                |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | port           | *DIGIT                                             |
      // |                | (RFC 3986 §3.2.3)                                  |
      // | userinfo       | *( unreserved / pct-encoded / sub-delims / ":" )   |
      // |                | (RFC 3986 §3.2.1) : forbidden in HTTP              |
      // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"              |
      // |                | (RFC 3986 §2.3)                                    |
      // | pct-encoded    | "%" HEXDIG HEXDIG                                  |
      // |                | (RFC 3986 §2.1)                                    |
      // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")"                  |
      // |                | "*" / "+" / "," / ";" / "="                        |
      // |                | (RFC 3986 §2.2)                                    |
      // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet|
      // |                | (RFC 3986 §3.2.2)                                  |
      // | dec-octet      | DIGIT                 ; 0-9                        |
      // |                | / %x31-39 DIGIT       ; 10-99                      |
      // |                | / "1" 2DIGIT          ; 100-199                    |
      // |                | / "2" %x30-34 DIGIT   ; 200-249                    |
      // |                | / "25" %x30-35        ; 250-255                    |
      // |                | (RFC 3986 §3.2.2)                                  |
      // | reg-name       | *( unreserved / pct-encoded / sub-delims )         |
      // |                | (RFC 3986 §3.2.2)                                  |
      // +----------------+----------------------------------------------------+
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // [to-do] -> add support for this!
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    } else if (method_ == method::kOptions) {
      // +---------------------------------------------------------------------+
      // |                                                       asterisk-form |
      // +----------------+----------------------------------------------------+
      // | Field          | Definition                                         |
      // +----------------+----------------------------------------------------+
      // | asterisk-form  | "*"                                                |
      // +----------------+----------------------------------------------------+
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // [to-do] -> add support for this!
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    } else if (auto path = try_get_absolute_path(i, rln_end_.value()); path) {
      // +-------------------------------------------------------------------+
      // |                                                       origin-form |
      // +----------------+--------------------------------------------------+
      // | Field          | Definition                                       |
      // +----------------+--------------------------------------------------+
      // | path           | segment *( "/" segment )                         |
      // | segment        | *pchar                                           |
      // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"|
      // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"            |
      // | pct-encoded    | "%" HEXDIG HEXDIG                                |
      // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /  |
      // |                | "," / ";" / "="                                  |
      // +----------------+--------------------------------------------------+
      // | source: https://datatracker.ietf.org/doc/html/rfc9110             |
      // +----------------+--------------------------------------------------+
      target_ = target::kOriginForm;
      abs_path_ = path.value();
      if (auto query = try_get_query_part(i, rln_end_.value()); query) {
        query_ = query.value();
      }
    } else {
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // [to-do] -> add support for this!
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      // absolute-form
      // parse it as URI..
    }
    pth_end_ = i++;
    // +----------------+------------------------------------------------------+
    // | HTTP-version   | HTTP-name "/" DIGIT "." DIGIT                        |
    // | HTTP-name      | %s"HTTP"                                             |
    // +----------------+------------------------------------------------------+
    // | source: https://datatracker.ietf.org/doc/html/rfc9110 |               |
    // +-----------------------------------------------------------------------+
    if ((rln_end_.value() - i) < kHttpVersionLen) return false;
    if (buf_[i] != constants::character::kHUpperCase ||
        buf_[i + 1] != constants::character::kTUpperCase ||
        buf_[i + 2] != constants::character::kTUpperCase ||
        buf_[i + 3] != constants::character::kPUpperCase ||
        buf_[i + 4] != constants::character::kSlash ||
        !helpers::is_digit(buf_[i + 5]) ||
        buf_[i + 6] != constants::character::kDot ||
        !helpers::is_digit(buf_[i + 7])) {
      return false;
    }
    ver_end_ = i + 8;
    version_ = std::string_view((const char*)&buf_[pth_end_.value() + 1],
                                ver_end_.value() - pth_end_.value() - 1);
    return true;
  }
  // +-----------------+-------------------------------------------------------+
  // | Rule            | Definition                                            |
  // +-----------------+-------------------------------------------------------+
  // | header-field    | field-name ":" OWS field-value OWS                    |
  // | field-name      | token                                                 |
  // | token           | 1*tchar                                               |
  // | tchar           | ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" /   |
  // |                 | "*" / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"   |
  // | field-value     | *( field-content )                                    |
  // | field-content   | field-vchar [ *( SP / HTAB ) field-vchar ]            |
  // | field-vchar     | VCHAR / obs-text                                      |
  // | OWS             | *( SP / HTAB )                                        |
  // | obs-fold        | CRLF 1*( SP / HTAB ) ; obsolete, not supported        |
  // +-----------------+-------------------------------------------------------+
  // | source: https://datatracker.ietf.org/doc/html/rfc9110                   |
  // +-------------------------------------------------------------------------+
  bool check_headers() {
    std::size_t clength_count = 0;
    std::size_t tencoding_count = 0;
    std::size_t i = ver_end_.value() + sizeof(constants::string::kCrLf) - 1;
    std::size_t end = hdr_end_.value() + 2;
    int host_count = 0;
    while (i < end) {
      std::size_t beg = i;
      while (i < end && buf_[i] != constants::character::kColon) {
        buf_[i++] = helpers::tolower_ascii(buf_[i]);
      }
      if (i >= end) return false;
      std::size_t colon = i;
      while (i < end && buf_[i] != constants::character::kCr) i++;
      if (i >= end) return false;
      if (++i >= end || buf_[i] != constants::character::kLf) return false;
      std::string_view name((const char*)&buf_[beg], colon - beg);
      std::string_view value((const char*)&buf_[colon + 1], (i - 2) - colon);
      // [field-name] validation..
      if (!name.length()) return false;
      for (auto j = 0; j < name.length(); j++) {
        if (!helpers::is_token(name[j])) return false;
      }
      // [field-value] validation..
      for (auto k = 0; k < value.length(); k++) {
        if (!(helpers::is_vchar(value[k]) || helpers::is_obs_text(value[k]) ||
              value[k] == constants::character::kSpace ||
              value[k] == constants::character::kHTab)) {
          return false;
        }
      }
      // [checks] needed by protocol..
      if (!name.compare(headers::kContentLength)) {
        if ((++clength_count > 1) || (tencoding_count > 0)) return false;
        auto ows_value = helpers::ows_ltrim(helpers::ows_rtrim(value));
        if (!ows_value.length() || !helpers::is_digit(ows_value)) return false;
        auto first = ows_value.data();
        auto last = ows_value.data() + ows_value.size();
        auto [p, e] = std::from_chars(first, last, body_size_expected_);
        if (e != std::errc{}) return false;
        body_processing_ = body_size_expected_ > 0;
      } else if (!name.compare(headers::kTransferEncoding)) {
        if ((++tencoding_count > 1) || (clength_count > 0)) return false;
      } else if (!name.compare(headers::kHost)) {
        host_count++;
      }
      i++;
    }
    return host_count == 1;
  }
  std::optional<std::string> try_get_absolute_path(std::size_t& i,
                                                   std::size_t len) {
    std::string path;
    char current_character;
    if (buf_[i] != constants::character::kSlash) return std::nullopt;
    while (i < len) {
      if (buf_[i] == constants::character::kPercent) {
        if (i + 2 >= len) return std::nullopt;
        if (!helpers::is_hex_digit(buf_[i + 1]) ||
            !helpers::is_hex_digit(buf_[i + 2])) {
          return std::nullopt;
        }
        current_character = static_cast<char>(
            std::stoi(std::string((const char*)&buf_[i + 1], 2), nullptr, 16));
        i += 3;
      } else {
        current_character = buf_[i];
        if (current_character == constants::character::kSpace ||
            current_character == constants::character::kQuestion) {
          break;
        }
      }
      if (!helpers::is_pchar(current_character) &&
          current_character != constants::character::kSlash) {
        return std::nullopt;
      }
      path.push_back(current_character);
      i++;
    }
    return path;
  }
  std::optional<std::string> try_get_query_part(std::size_t& i,
                                                std::size_t len) {
    std::string query;
    char current_character;
    if (buf_[i] != constants::character::kQuestion) return std::nullopt;
    while (i < len) {
      if (buf_[i] == constants::character::kPercent) {
        if (i + 2 >= len) return std::nullopt;
        if (!helpers::is_hex_digit(buf_[i + 1]) ||
            !helpers::is_hex_digit(buf_[i + 2])) {
          return std::nullopt;
        }
        current_character = static_cast<char>(
            std::stoi(std::string((const char*)&buf_[i + 1], 2), nullptr, 16));
        i += 3;
      } else {
        current_character = buf_[i];
        if (current_character == constants::character::kSpace) break;
      }
      if (!helpers::is_pchar(current_character) &&
          current_character != constants::character::kSlash &&
          current_character != constants::character::kQuestion) {
        return std::nullopt;
      }
      query.push_back(current_character);
      i++;
    }
    return query;
  }
  std::optional<std::size_t> get(const char* const str, std::size_t str_len,
                                 const char* const pattern,
                                 std::size_t pattern_len) {
    for (std::size_t i = 0; i < str_len; ++i) {
      std::size_t j = 0;
      while (j < pattern_len) {
        if (i + j == str_len) return {};
        if (str[i + j] != pattern[j]) break;
        j++;
      }
      if (j == pattern_len) return i;
    }
    return {};
  }
  // ___________________________________________________________________________
  // STATIC-ATTRIBUTEs                                               ( private )
  //
  std::size_t buf_sz_;
  std::size_t bod_sz_;
  std::size_t slh_sz_;
  std::size_t cur_;
  char* buf_;
  method method_;
  target target_;
  std::string abs_path_;
  std::string query_;
  std::string version_;
  bool body_processing_;
  std::size_t body_size_expected_;
  std::size_t body_size_received_;
  std::shared_ptr<request> request_;
  std::optional<std::size_t> hdr_end_;  // headers section end.
  std::optional<std::size_t> rln_end_;  // request line end.
  std::optional<std::size_t> mtd_end_;  // method end.
  std::optional<std::size_t> pth_end_;  // path end.
  std::optional<std::size_t> ver_end_;  // http version end.
};
}  // namespace martianlabs::doba::protocol::http11

#endif
