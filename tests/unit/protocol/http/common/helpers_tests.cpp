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

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "protocol/http/common/helpers.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::deserialization_status;
using martianlabs::doba::protocol::http::helpers;
}  // namespace

// +===========================================================================+
// | [>] character predicates cover every possible byte          ( test-case ) |
// +===========================================================================+
DOBA_TEST("character predicates cover every possible byte") {
  for (unsigned int value = 0; value <= 0xff; value++) {
    const auto c = static_cast<unsigned char>(value);
    const bool digit = c >= '0' && c <= '9';
    const bool alpha = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    const bool hex = digit || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    const bool token =
        alpha || digit ||
        std::string_view("!#$%&'*+-.^_`|~").find(static_cast<char>(c)) !=
            std::string_view::npos;
    const bool unreserved = alpha || digit ||
                            std::string_view("-._~").find(
                                static_cast<char>(c)) != std::string_view::npos;
    const bool sub_delim =
        std::string_view("!$&'()*+,;=").find(static_cast<char>(c)) !=
        std::string_view::npos;
    const bool pchar = unreserved || sub_delim || c == ':' || c == '@';
    const bool vchar = c >= 0x21 && c <= 0x7e;
    const bool obs_text = c >= 0x80;
    const bool qdtext = c == 0x09 || c == 0x20 || c == 0x21 ||
                        (c >= 0x23 && c <= 0x5b) || (c >= 0x5d && c <= 0x7e) ||
                        obs_text;
    const bool ctext = c == 0x09 || c == 0x20 || (c >= 0x21 && c <= 0x27) ||
                       (c >= 0x2a && c <= 0x5b) || (c >= 0x5d && c <= 0x7e) ||
                       obs_text;
    const bool atext =
        alpha || digit ||
        std::string_view("!#$%&'*+-/=?^_`{|}~").find(static_cast<char>(c)) !=
            std::string_view::npos;
    const bool dtext =
        (c >= 0x21 && c <= 0x5a) || (c >= 0x5e && c <= 0x7e) || obs_text;
    const bool etagc = c == 0x21 || (c >= 0x23 && c <= 0x7e) || obs_text;
    const bool cookie = c == 0x21 || (c >= 0x23 && c <= 0x2b) ||
                        (c >= 0x2d && c <= 0x3a) || (c >= 0x3c && c <= 0x5b) ||
                        (c >= 0x5d && c <= 0x7e);
    const bool cookie_av = c >= 0x20 && c <= 0x7e && c != ';';
    const bool ows = c == 0x20 || c == 0x09;
    DOBA_EXPECT_EQUAL(helpers::is_digit(c), digit);
    DOBA_EXPECT_EQUAL(helpers::is_hex_digit(c), hex);
    DOBA_EXPECT_EQUAL(helpers::is_alpha(c), alpha);
    DOBA_EXPECT_EQUAL(helpers::is_token(c), token);
    DOBA_EXPECT_EQUAL(helpers::is_unreserved(c), unreserved);
    DOBA_EXPECT_EQUAL(helpers::is_sub_delim(c), sub_delim);
    DOBA_EXPECT_EQUAL(helpers::is_pchar(c), pchar);
    DOBA_EXPECT_EQUAL(helpers::is_vchar(c), vchar);
    DOBA_EXPECT_EQUAL(helpers::is_qdtext(c), qdtext);
    DOBA_EXPECT_EQUAL(helpers::is_obs_text(c), obs_text);
    DOBA_EXPECT_EQUAL(helpers::is_ctext(c), ctext);
    DOBA_EXPECT_EQUAL(helpers::is_atext(c), atext);
    DOBA_EXPECT_EQUAL(helpers::is_dtext(c), dtext);
    DOBA_EXPECT_EQUAL(helpers::is_etagc(c), etagc);
    DOBA_EXPECT_EQUAL(helpers::is_cookie_octet(c), cookie);
    DOBA_EXPECT_EQUAL(helpers::is_cookie_av_octet(c), cookie_av);
    DOBA_EXPECT_EQUAL(helpers::is_ows(c), ows);
    DOBA_EXPECT_EQUAL(helpers::tolower_ascii(c),
                      c >= 'A' && c <= 'Z' ? c + 32 : c);
    DOBA_EXPECT_EQUAL(helpers::is_digit(static_cast<char>(c)), digit);
    DOBA_EXPECT_EQUAL(helpers::is_hex_digit(static_cast<char>(c)), hex);
    DOBA_EXPECT_EQUAL(helpers::is_token(static_cast<char>(c)), token);
  }
}
// +===========================================================================+
// | [>] decimal helpers reject invalid and overflow values      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decimal helpers reject empty invalid and overflow values") {
  constexpr std::string_view digits[] = {"0", "00", "1", "1234567890"};
  for (const auto value : digits) DOBA_EXPECT(helpers::is_digits(value));
  constexpr std::string_view not_digits[] = {
      "", "+1", "-1", " 1", "1 ", "1.0", "1x", "\x80",
  };
  for (const auto value : not_digits) {
    DOBA_EXPECT(!helpers::is_digits(value));
  }
  std::size_t parsed = 7;
  DOBA_EXPECT(helpers::parse_size_t("0", parsed));
  DOBA_EXPECT_EQUAL(parsed, 0);
  constexpr auto maximum = std::numeric_limits<std::size_t>::max();
  const std::string max_text = std::to_string(maximum);
  DOBA_EXPECT(helpers::parse_size_t(max_text, parsed));
  DOBA_EXPECT_EQUAL(parsed, maximum);
  std::string overflow = max_text;
  overflow += '0';
  DOBA_EXPECT(!helpers::parse_size_t(overflow, parsed));
  for (const auto value : not_digits) {
    DOBA_EXPECT(!helpers::parse_size_t(value, parsed));
  }
}
// +===========================================================================+
// | [>] token entity tag and quality grammars cover boundaries  ( test-case ) |
// +===========================================================================+
DOBA_TEST("token entity tag and quality grammars cover boundaries") {
  constexpr std::string_view tokens[] = {
      "a", "A-Z", "0123", "!#$%&'*+-.^_`|~", "token",
  };
  for (const auto value : tokens) DOBA_EXPECT(helpers::is_token(value));
  constexpr std::string_view invalid_tokens[] = {
      "", "a b", "a\tb", "a,b", "a/b", "\x80",
  };
  for (const auto value : invalid_tokens) {
    DOBA_EXPECT(!helpers::is_token(value));
  }
  constexpr std::string_view tags[] = {
      "\"\"", "\"tag\"", "W/\"tag\"", "\"!\"", "\"\x80\"",
  };
  for (const auto value : tags) DOBA_EXPECT(helpers::is_entity_tag(value));
  constexpr std::string_view invalid_tags[] = {
      "",      "tag",    "w/\"tag\"", "W/tag",
      "\"tag", "\"a\"x", "\"a b\"",   "\"a\x7f\"",
  };
  for (const auto value : invalid_tags) {
    DOBA_EXPECT(!helpers::is_entity_tag(value));
  }
  constexpr std::string_view qvalues[] = {
      "0",     "0.", "0.0", "0.00", "0.000", "0.001",
      "0.999", "1",  "1.",  "1.0",  "1.00",  "1.000",
  };
  for (const auto value : qvalues) DOBA_EXPECT(helpers::is_qvalue(value));
  constexpr std::string_view invalid_qvalues[] = {
      "", ".5", "00", "0.0000", "1.001", "1.0000", "2", "0.a",
  };
  for (const auto value : invalid_qvalues) {
    DOBA_EXPECT(!helpers::is_qvalue(value));
  }
  constexpr std::string_view weights[] = {
      ";q=0",
      ";Q=1",
      " ; q=0.5",
      "\t;\tq=1.000",
  };
  for (const auto value : weights) DOBA_EXPECT(helpers::consume_weight(value));
  constexpr std::string_view invalid_weights[] = {
      "", "q=1", "; q =1", "; q= 1", ";x=1", ";q=", ";q=1.1",
  };
  for (const auto value : invalid_weights) {
    DOBA_EXPECT(!helpers::consume_weight(value));
  }
}
// +===========================================================================+
// | [>] whitespace trimming preserves the referenced buffer     ( test-case ) |
// +===========================================================================+
DOBA_TEST("whitespace trimming preserves the referenced buffer") {
  constexpr std::string_view cases[] = {
      "", "value", " value", "value ", "\tvalue\t", " \tvalue\t ", " \t ",
  };
  constexpr std::string_view expected[] = {
      "", "value", "value", "value", "value", "value", "",
  };
  for (std::size_t i = 0; i < std::size(cases); i++) {
    std::string_view value = cases[i];
    helpers::ows_trim(value);
    DOBA_EXPECT_EQUAL(value, expected[i]);
  }
  std::string_view value = " \tvalue\t ";
  helpers::ows_ltrim(value);
  DOBA_EXPECT_EQUAL(value, "value\t ");
  helpers::ows_rtrim(value);
  DOBA_EXPECT_EQUAL(value, "value");
}
// +===========================================================================+
// | [>] IP address validators cover canonical forms             ( test-case ) |
// +===========================================================================+
DOBA_TEST("IP address validators cover canonical forms") {
  constexpr std::string_view dec_octets[] = {
      "0", "9", "10", "99", "100", "199", "200", "249", "250", "255",
  };
  for (const auto value : dec_octets) DOBA_EXPECT(helpers::is_dec_octet(value));
  constexpr std::string_view invalid_octets[] = {
      "", "00", "01", "000", "09", "256", "999", "1x", "-1",
  };
  for (const auto value : invalid_octets) {
    DOBA_EXPECT(!helpers::is_dec_octet(value));
  }
  constexpr std::string_view ipv4[] = {
      "0.0.0.0",
      "127.0.0.1",
      "192.168.1.1",
      "255.255.255.255",
  };
  for (const auto value : ipv4) DOBA_EXPECT(helpers::is_ip_v4_address(value));
  constexpr std::string_view invalid_ipv4[] = {
      "",         "127",      "127.0.0",   "127.0.0.1.", ".127.0.0.1",
      "127..0.1", "01.0.0.1", "256.0.0.1", "a.0.0.1",
  };
  for (const auto value : invalid_ipv4) {
    DOBA_EXPECT(!helpers::is_ip_v4_address(value));
  }
  constexpr std::string_view ipv6[] = {
      "::",
      "::1",
      "1::",
      "2001:db8::1",
      "2001:db8:0:0:0:0:2:1",
      "::ffff:192.0.2.1",
  };
  for (const auto value : ipv6) DOBA_EXPECT(helpers::is_ip_v6_address(value));
  constexpr std::string_view invalid_ipv6[] = {
      "",
      ":",
      "1:2:3",
      "1::2::3",
      "12345::",
      "gggg::1",
      "1:2:3:4:5:6:7:8:9",
      "::ffff:256.0.0.1",
  };
  for (const auto value : invalid_ipv6) {
    DOBA_EXPECT(!helpers::is_ip_v6_address(value));
  }
  DOBA_EXPECT(helpers::is_ip_v_future("v1.a"));
  DOBA_EXPECT(helpers::is_ip_v_future("VF.a:b"));
  DOBA_EXPECT(!helpers::is_ip_v_future(""));
  DOBA_EXPECT(!helpers::is_ip_v_future("v.a"));
  DOBA_EXPECT(!helpers::is_ip_v_future("v1."));
  DOBA_EXPECT(helpers::is_ip_literal("[::1]"));
  DOBA_EXPECT(helpers::is_ip_literal("[v1.a]"));
  DOBA_EXPECT(!helpers::is_ip_literal("::1"));
  DOBA_EXPECT(!helpers::is_ip_literal("[::1"));
}
// +===========================================================================+
// | [>] host validators classify every URI host alternative     ( test-case ) |
// +===========================================================================+
DOBA_TEST("host validators classify every URI host alternative") {
  using host_type = helpers::host_type;
  DOBA_EXPECT_EQUAL(helpers::check_uri_host(""), host_type::kRegName);
  DOBA_EXPECT_EQUAL(helpers::check_uri_host("example.com"),
                    host_type::kRegName);
  DOBA_EXPECT_EQUAL(helpers::check_uri_host("127.0.0.1"),
                    host_type::kIpV4Address);
  DOBA_EXPECT_EQUAL(helpers::check_uri_host("[::1]"), host_type::kIpLiteral);
  DOBA_EXPECT_EQUAL(helpers::check_uri_host("bad host"), host_type::kUnknown);
  constexpr std::string_view reg_names[] = {
      "", "example.com", "a-b_c~d", "a%20b", "!$&'()*+,;=",
  };
  for (const auto value : reg_names) DOBA_EXPECT(helpers::is_reg_name(value));
  constexpr std::string_view invalid_reg_names[] = {
      "a b", "a%", "a%2", "a%2x", "a/b", "a:b", "\x80",
  };
  for (const auto value : invalid_reg_names) {
    DOBA_EXPECT(!helpers::is_reg_name(value));
  }
  struct test_case {
    std::string_view source;
    std::string_view host;
    std::string_view port;
    host_type type;
  };
  constexpr test_case valid[] = {
      {"", "", "", host_type::kRegName},
      {"example.com", "example.com", "", host_type::kRegName},
      {"example.com:80", "example.com", "80", host_type::kRegName},
      {"127.0.0.1:0", "127.0.0.1", "0", host_type::kIpV4Address},
      {"[::1]:443", "[::1]", "443", host_type::kIpLiteral},
      {"host:", "host", "", host_type::kRegName},
  };
  for (const auto& test : valid) {
    std::string_view host;
    std::string_view port;
    host_type type = host_type::kUnknown;
    DOBA_EXPECT(helpers::check_host_port(test.source, host, port, type));
    DOBA_EXPECT_EQUAL(host, test.host);
    DOBA_EXPECT_EQUAL(port, test.port);
    DOBA_EXPECT_EQUAL(type, test.type);
    DOBA_EXPECT(helpers::check_host_port(test.source));
  }
  constexpr std::string_view invalid[] = {
      "bad host", "host:x", "[::1", "[::1]x", "[::1]:x",
  };
  for (const auto value : invalid) {
    DOBA_EXPECT(!helpers::check_host_port(value));
  }
}
// +===========================================================================+
// | [>] token68 and base64 validators enforce padding           ( test-case ) |
// +===========================================================================+
DOBA_TEST("token68 and base64 validators enforce padding") {
  constexpr std::string_view token68[] = {
      "a", "abc", "a-b_c.d~e+f/g", "abc=", "abc==",
  };
  for (const auto value : token68) DOBA_EXPECT(helpers::is_token68(value));
  constexpr std::string_view invalid_token68[] = {
      "", "=", "a=b", "a b", "a,", "\x80",
  };
  for (const auto value : invalid_token68) {
    DOBA_EXPECT(!helpers::is_token68(value));
  }
  constexpr std::string_view base64[] = {
      "Zg==",
      "Zm8=",
      "Zm9v",
      "YW55IGNhcm5hbCBwbGVhc3VyZS4=",
  };
  for (const auto value : base64) {
    DOBA_EXPECT(helpers::check_base64_value(value));
  }
  constexpr std::string_view invalid_base64[] = {
      "", "Z", "Zg", "Zg=", "=m9v", "Zm=9", "Zm9v=", "Zm9v\n",
  };
  for (const auto value : invalid_base64) {
    DOBA_EXPECT(!helpers::check_base64_value(value));
  }
}
// +===========================================================================+
// | [>] token string and comment consumers stop at boundaries   ( test-case ) |
// +===========================================================================+
DOBA_TEST("token string and comment consumers stop at boundaries") {
  DOBA_EXPECT_EQUAL(helpers::consume_token("token rest"), "token");
  DOBA_EXPECT(helpers::consume_token("").empty());
  DOBA_EXPECT(helpers::consume_token(" token").empty());
  DOBA_EXPECT_EQUAL(helpers::consume_quoted_string("\"a,b\"rest"), "\"a,b\"");
  DOBA_EXPECT_EQUAL(helpers::consume_quoted_string("\"a\\\"b\"rest"),
                    "\"a\\\"b\"");
  DOBA_EXPECT(helpers::consume_quoted_string("").empty());
  DOBA_EXPECT(helpers::consume_quoted_string("\"unterminated").empty());
  DOBA_EXPECT(helpers::consume_quoted_string("\"bad\x01\"").empty());
  DOBA_EXPECT_EQUAL(helpers::consume_comment("()rest"), "()");
  DOBA_EXPECT_EQUAL(helpers::consume_comment("(a(b)c)rest"), "(a(b)c)");
  DOBA_EXPECT_EQUAL(helpers::consume_comment("(a\\)b)rest"), "(a\\)b)");
  DOBA_EXPECT(helpers::consume_comment("").empty());
  DOBA_EXPECT(helpers::consume_comment("(unterminated").empty());
  DOBA_EXPECT(helpers::consume_comment("(bad\x01)").empty());
  DOBA_EXPECT_EQUAL(helpers::consume_token_or_quoted_string("token rest"),
                    "token");
  DOBA_EXPECT_EQUAL(helpers::consume_token_or_quoted_string("\"a b\"rest"),
                    "\"a b\"");
}
// +===========================================================================+
// | [>] mailbox helpers validate address grammar                ( test-case ) |
// +===========================================================================+
DOBA_TEST("mailbox helpers validate address grammar") {
  constexpr std::string_view valid[] = {
      "a@example.com",
      "first.last@example.com",
      "\"first last\"@example.com",
      "a@[127.0.0.1]",
      "(a)a(b)@(c)example.com(d)",
  };
  for (const auto value : valid) DOBA_EXPECT(helpers::check_mailbox(value));
  constexpr std::string_view invalid[] = {
      "",
      "a",
      "@example.com",
      "a@",
      ".a@example.com",
      "a..b@example.com",
      "a@example..com",
      "a@[unterminated",
      "a@example.com trailing",
  };
  for (const auto value : invalid) {
    DOBA_EXPECT(!helpers::check_mailbox(value));
  }
  std::size_t i = 0;
  DOBA_EXPECT(helpers::consume_cfws(" \t(comment)value", i));
  DOBA_EXPECT_EQUAL(i, 11);
  i = 0;
  DOBA_EXPECT(!helpers::consume_cfws("(unterminated", i));
  i = 0;
  DOBA_EXPECT(helpers::consume_dot_atom("a.b rest", i));
  DOBA_EXPECT_EQUAL(i, 4);
  i = 0;
  DOBA_EXPECT(!helpers::consume_dot_atom("a..b", i));
  i = 0;
  DOBA_EXPECT(helpers::consume_domain_literal("[127.0.0.1]rest", i));
  DOBA_EXPECT_EQUAL(i, 11);
  i = 0;
  DOBA_EXPECT(helpers::consume_addr_spec("a@example.com", i));
  DOBA_EXPECT_EQUAL(i, 13);
  i = 0;
  DOBA_EXPECT(helpers::consume_word("atom rest", i));
  DOBA_EXPECT_EQUAL(i, 5);
}
// +===========================================================================+
// | [>] cookie and directive validators enforce complete values ( test-case ) |
// +===========================================================================+
DOBA_TEST("cookie and directive validators enforce complete values") {
  constexpr std::string_view cookie_values[] = {
      "", "value", "\"\"", "\"value\"", "!#$%&'()*+-./:<=>?@[]^_`{|}~",
  };
  for (const auto value : cookie_values) {
    DOBA_EXPECT(helpers::is_cookie_value(value));
  }
  constexpr std::string_view invalid_cookie_values[] = {
      "a b", "a;b", "a,b", "\"unterminated", "\"a b\"", "\x80",
  };
  for (const auto value : invalid_cookie_values) {
    DOBA_EXPECT(!helpers::is_cookie_value(value));
  }
  DOBA_EXPECT(helpers::is_cookie_pair("name=value"));
  DOBA_EXPECT(helpers::is_cookie_pair("name="));
  DOBA_EXPECT(!helpers::is_cookie_pair("=value"));
  DOBA_EXPECT(!helpers::is_cookie_pair("name"));
  DOBA_EXPECT(helpers::is_cookie_av("name=value"));
  DOBA_EXPECT(helpers::is_cookie_av("name"));
  DOBA_EXPECT(!helpers::is_cookie_av("name;value"));
  constexpr std::string_view directives[] = {
      "no-cache",
      "max-age=60",
      "name=\"quoted value\"",
  };
  for (const auto value : directives) DOBA_EXPECT(helpers::is_directive(value));
  constexpr std::string_view invalid_directives[] = {
      "", "=value", "name=", "name =value", "name=value trailing",
  };
  for (const auto value : invalid_directives) {
    DOBA_EXPECT(!helpers::is_directive(value));
  }
}
// +===========================================================================+
// | [>] parameter and list iterators enforce boundaries         ( test-case ) |
// +===========================================================================+
DOBA_TEST("parameter and list iterators enforce consumer boundaries") {
  std::size_t bytes = 99;
  DOBA_EXPECT(helpers::consume_parameter("name=value;next=x", bytes, false));
  DOBA_EXPECT_EQUAL(bytes, 10);
  DOBA_EXPECT(helpers::consume_parameter("name = \"value\"", bytes, true));
  DOBA_EXPECT_EQUAL(bytes, 14);
  DOBA_EXPECT(!helpers::consume_parameter("name = value", bytes, false));
  DOBA_EXPECT_EQUAL(bytes, 0);
  DOBA_EXPECT(helpers::consume_extension_parameter("name;next", bytes));
  DOBA_EXPECT_EQUAL(bytes, 4);
  DOBA_EXPECT(helpers::consume_extension_parameter("name=value", bytes));
  DOBA_EXPECT_EQUAL(bytes, 10);
  DOBA_EXPECT(!helpers::consume_extension_parameter("name=", bytes));
  std::size_t parameters = 0;
  auto consume_parameter = [&parameters](std::string_view value,
                                         std::size_t& used) {
    if (!helpers::consume_parameter(value, used, false)) return false;
    parameters++;
    return true;
  };
  DOBA_EXPECT(
      helpers::for_each_parameter(";a=1 ; b=2", true, consume_parameter));
  DOBA_EXPECT_EQUAL(parameters, 2);
  DOBA_EXPECT(!helpers::for_each_parameter(";", true, consume_parameter));
  DOBA_EXPECT(helpers::for_each_parameter(";;", false, consume_parameter));
  std::vector<std::string_view> elements;
  DOBA_EXPECT(helpers::for_each_list_element(
      " , token, \"a,b\",,last ", [&elements](std::string_view element) {
        elements.push_back(element);
        return true;
      }));
  DOBA_EXPECT_EQUAL(elements.size(), 3);
  DOBA_EXPECT_EQUAL(elements[0], "token");
  DOBA_EXPECT_EQUAL(elements[1], "\"a,b\"");
  DOBA_EXPECT_EQUAL(elements[2], "last ");
  DOBA_EXPECT(!helpers::for_each_list_element(
      "\"unterminated", [](std::string_view) { return true; }));
  DOBA_EXPECT(!helpers::for_each_list_element(
      "a", [](std::string_view) { return false; }));
}
// +===========================================================================+
// | [>] authentication and product grammars cover alternatives  ( test-case ) |
// +===========================================================================+
DOBA_TEST("authentication and product grammars cover alternatives") {
  constexpr std::string_view auth_params[] = {
      "", "name=value", "name = \"value\"", "a=1, b=2", ",,a=1,,",
  };
  for (const auto value : auth_params) {
    DOBA_EXPECT(helpers::check_auth_param_list(value));
  }
  constexpr std::string_view credentials[] = {
      "Basic",
      "Basic ",
      "Basic Zm9v",
      "Basic name=",
      "Digest realm=example",
      "Digest realm=\"example\", qop=auth",
  };
  for (const auto value : credentials) {
    DOBA_EXPECT(helpers::check_credentials(value));
  }
  constexpr std::string_view invalid_credentials[] = {
      "",
      " Basic",
      "Basic\tvalue",
  };
  for (const auto value : invalid_credentials) {
    DOBA_EXPECT(!helpers::check_credentials(value));
  }
  DOBA_EXPECT(!helpers::challenge_opens_param_list("Basic"));
  DOBA_EXPECT(!helpers::challenge_opens_param_list("Basic Zm9v"));
  DOBA_EXPECT(helpers::challenge_opens_param_list("Digest realm=example"));
  DOBA_EXPECT(helpers::check_challenge_list(
      "Basic realm=one, charset=UTF-8, Bearer token"));
  DOBA_EXPECT(!helpers::check_challenge_list("realm=one"));
  DOBA_EXPECT_EQUAL(helpers::consume_product("doba/1.0 rest"), "doba/1.0");
  DOBA_EXPECT_EQUAL(helpers::consume_product("doba rest"), "doba");
  DOBA_EXPECT(helpers::consume_product("doba/").empty());
  DOBA_EXPECT(helpers::check_product_list("doba/1.0 (comment) lib/2"));
  DOBA_EXPECT(!helpers::check_product_list(""));
  DOBA_EXPECT(!helpers::check_product_list(" doba"));
  DOBA_EXPECT(!helpers::check_product_list("doba "));
}
// +===========================================================================+
// | [>] URI component validators enforce RFC 3986 syntax        ( test-case ) |
// +===========================================================================+
DOBA_TEST("URI component validators enforce RFC 3986 syntax") {
  constexpr std::string_view schemes[] = {
      "http", "HTTPS", "a", "git+ssh", "a-b.c1",
  };
  for (const auto value : schemes) DOBA_EXPECT(helpers::is_uri_scheme(value));
  constexpr std::string_view invalid_schemes[] = {
      "", "1http", "+http", "http:", "http/", "http space",
  };
  for (const auto value : invalid_schemes) {
    DOBA_EXPECT(!helpers::is_uri_scheme(value));
  }
  std::size_t i = 7;
  DOBA_EXPECT(helpers::consume_uri_scheme("http://host", i));
  DOBA_EXPECT_EQUAL(i, 5);
  i = 7;
  DOBA_EXPECT(!helpers::consume_uri_scheme("/path", i));
  DOBA_EXPECT_EQUAL(i, 7);
  i = 0;
  DOBA_EXPECT(helpers::consume_uri_pct_encoded("%2F", i));
  DOBA_EXPECT_EQUAL(i, 3);
  i = 0;
  DOBA_EXPECT(!helpers::consume_uri_pct_encoded("%2", i));
  i = 0;
  DOBA_EXPECT(!helpers::consume_uri_pct_encoded("%2x", i));
  DOBA_EXPECT(helpers::percent_decode_validate("a%20b"));
  DOBA_EXPECT(!helpers::percent_decode_validate("a%00b"));
  std::string storage = "a%20b%2Fc";
  std::string_view decoded = storage;
  helpers::percent_decode_in_place(decoded);
  DOBA_EXPECT_EQUAL(decoded, "a b/c");
  DOBA_EXPECT_EQUAL(decoded.data(), storage.data());
  DOBA_EXPECT(helpers::check_uri_userinfo("user:pass"));
  DOBA_EXPECT(helpers::check_uri_userinfo("user%20name"));
  DOBA_EXPECT(!helpers::check_uri_userinfo("user@host"));
  DOBA_EXPECT(helpers::check_serialized_origin("https://example.com:443"));
  DOBA_EXPECT(!helpers::check_serialized_origin("https://example.com/"));
  DOBA_EXPECT(!helpers::check_serialized_origin("//example.com"));
  DOBA_EXPECT(helpers::check_uri_authority("user@example.com:80"));
  DOBA_EXPECT(helpers::check_uri_authority("[::1]:443"));
  DOBA_EXPECT(!helpers::check_uri_authority("user@@example.com"));
}
// +===========================================================================+
// | [>] URI references cover paths queries and fragments        ( test-case ) |
// +===========================================================================+
DOBA_TEST("URI references cover relative absolute query and fragment") {
  constexpr std::string_view valid[] = {
      "",
      "/",
      "/path",
      "path",
      "../path",
      "?query",
      "#fragment",
      "http://example.com/path?query#fragment",
      "//example.com/path",
      "urn:example:test",
      "a:b/path",
      "/a%20b",
  };
  for (const auto value : valid) {
    DOBA_EXPECT(helpers::check_uri_reference(value, true));
  }
  DOBA_EXPECT(helpers::check_uri_reference("/path?query", false));
  DOBA_EXPECT(!helpers::check_uri_reference("/path#fragment", false));
  constexpr std::string_view invalid[] = {
      "a%", "a%2", "a%2x", "//bad host/path", "/path\x01", "http://[::1/path",
  };
  for (const auto value : invalid) {
    DOBA_EXPECT(!helpers::check_uri_reference(value, true));
  }
  DOBA_EXPECT_EQUAL(helpers::default_port_for_scheme("http"), "80");
  DOBA_EXPECT_EQUAL(helpers::default_port_for_scheme("HTTPS"), "443");
  DOBA_EXPECT(helpers::default_port_for_scheme("ftp").empty());
  DOBA_EXPECT(helpers::ports_equivalent("http", "", "80"));
  DOBA_EXPECT(helpers::ports_equivalent("https", "443", ""));
  DOBA_EXPECT(!helpers::ports_equivalent("http", "", "8080"));
  DOBA_EXPECT(helpers::ports_equivalent("ftp", "21", "21"));
}
// +===========================================================================+
// | [>] request target deserializers report precise consumption ( test-case ) |
// +===========================================================================+
DOBA_TEST("request target deserializers report precise consumption") {
  using host_type = helpers::host_type;
  std::string_view host;
  std::string_view port;
  host_type type = host_type::kUnknown;
  std::size_t used = 99;
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_authority_form(
                        "example.com:443 rest", host, port, type, used),
                    deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(host, "example.com");
  DOBA_EXPECT_EQUAL(port, "443");
  DOBA_EXPECT_EQUAL(type, host_type::kRegName);
  DOBA_EXPECT_EQUAL(used, 15);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_authority_form("", host, port, type, used),
      deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_authority_form(
                        "host", host, port, type, used),
                    deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_asterisk_form("", used),
                    deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_asterisk_form("/", used),
                    deserialization_status::kInvalidSource);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_asterisk_form("* rest", used),
      deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(used, 1);
  std::string_view path;
  std::string_view query;
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_origin_form(
                        "/a/b?q=1 rest", path, query, used),
                    deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(path, "/a/b");
  DOBA_EXPECT_EQUAL(query, "q=1");
  DOBA_EXPECT_EQUAL(used, 8);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_origin_form("", path, query, used),
      deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_origin_form("relative", path, query, used),
      deserialization_status::kInvalidSource);
  bool has_authority = false;
  std::string_view scheme;
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_absolute_form(
                        "http://example.com/a?q=1 rest", path, query,
                        has_authority, host, port, type, scheme, used),
                    deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(scheme, "http");
  DOBA_EXPECT(has_authority);
  DOBA_EXPECT_EQUAL(host, "example.com");
  DOBA_EXPECT_EQUAL(path, "/a");
  DOBA_EXPECT_EQUAL(query, "q=1");
  DOBA_EXPECT_EQUAL(used, 24);
}
// +===========================================================================+
// | [>] path and query parsers handle truncated buffers         ( test-case ) |
// +===========================================================================+
DOBA_TEST("path and query deserializers handle empty truncated buffers") {
  std::string_view value;
  std::size_t used = 99;
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_absolute_path("", value, used),
      deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(used, 0);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_absolute_path("relative", value, used),
      deserialization_status::kInvalidSource);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_absolute_path("/a%", value, used),
      deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_absolute_path("/a%2x", value, used),
      deserialization_status::kInvalidSource);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_absolute_path("/a/b rest", value, used),
      deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(value, "/a/b");
  DOBA_EXPECT_EQUAL(used, 4);
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_query("", value, used),
                    deserialization_status::kSucceeded);
  DOBA_EXPECT(value.empty());
  DOBA_EXPECT_EQUAL(used, 0);
  DOBA_EXPECT_EQUAL(
      helpers::try_to_deserialize_as_query("a/b?c rest", value, used),
      deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(value, "a/b?c");
  DOBA_EXPECT_EQUAL(used, 5);
  DOBA_EXPECT_EQUAL(helpers::try_to_deserialize_as_query("a%", value, used),
                    deserialization_status::kMoreBytesNeeded);
}
// +===========================================================================+
// | [>] case and query helpers preserve views                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("case and query helpers preserve views") {
  DOBA_EXPECT(helpers::iequals("Content-Type", "content-type"));
  DOBA_EXPECT(helpers::iequals("", ""));
  DOBA_EXPECT(!helpers::iequals("a", "b"));
  DOBA_EXPECT(!helpers::iequals("a", "aa"));
  std::array<std::string_view, 4> keys;
  std::array<std::string_view, 4> values;
  const std::size_t count =
      helpers::split_query_parameters("a=1&&b&=empty&c=3=4&", keys, values);
  DOBA_EXPECT_EQUAL(count, 4);
  DOBA_EXPECT_EQUAL(keys[0], "a");
  DOBA_EXPECT_EQUAL(values[0], "1");
  DOBA_EXPECT_EQUAL(keys[1], "b");
  DOBA_EXPECT(values[1].empty());
  DOBA_EXPECT(keys[2].empty());
  DOBA_EXPECT_EQUAL(values[2], "empty");
  DOBA_EXPECT_EQUAL(keys[3], "c");
  DOBA_EXPECT_EQUAL(values[3], "3=4");
  std::span<std::string_view> no_keys;
  std::span<std::string_view> no_values;
  DOBA_EXPECT_EQUAL(helpers::split_query_parameters("a=1", no_keys, no_values),
                    0);
}
