// Copyright 2026 martianLabs. Licensed under the Apache License, Version 2.0.
#ifndef martianlabs_doba_protocol_http_v11_static_file_helpers_h
#define martianlabs_doba_protocol_http_v11_static_file_helpers_h

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "common/date_server_helpers.h"
#include "protocol/http/common/headers/date.h"
#include "protocol/http/common/helpers.h"

namespace martianlabs::doba::protocol::http::v11::detail {
inline std::string static_file_date(std::int64_t seconds) {
  const auto time = static_cast<std::time_t>(seconds);
  std::tm value{};
#ifdef _WIN32
  if (common::detail::gm_time(&value, &time) != 0) return {};
#else
  if (!common::detail::gm_time(&value, &time)) return {};
#endif
  constexpr const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  constexpr const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char output[40];
  std::snprintf(output, sizeof(output), "%s, %02d %s %04d %02d:%02d:%02d GMT",
                days[value.tm_wday], value.tm_mday, months[value.tm_mon],
                value.tm_year + 1900, value.tm_hour, value.tm_min, value.tm_sec);
  return output;
}

// Accept IMF-fixdate and both obsolete HTTP-date formats. Do not use the
// process locale/time zone; reject impossible calendar dates.
inline std::optional<std::int64_t> static_file_time(std::string_view input) {
  if (!::martianlabs::doba::protocol::http::headers::date::check(input)) return {};
  const bool old = input.find('-') != input.npos;
  const char* format = old ? "%A, %d-%b-%y %H:%M:%S GMT" :
      (input.find(',') != input.npos ? "%a, %d %b %Y %H:%M:%S GMT" :
                                     "%a %b %d %H:%M:%S %Y");
  std::istringstream stream{std::string(input)};
  stream.imbue(std::locale::classic());
  std::tm value{};
  stream >> std::get_time(&value, format);
  if (stream.fail()) return {};
  using namespace std::chrono;
  if (old) {
    const auto current = year_month_day(floor<days>(system_clock::now()));
    int year_value = int(current.year()) / 100 * 100 + value.tm_year % 100;
    if (year_value > int(current.year()) + 50) year_value -= 100;
    value.tm_year = year_value - 1900;
  }
  const year_month_day date{year(value.tm_year + 1900),
                            month(static_cast<unsigned>(value.tm_mon + 1)),
                            day(static_cast<unsigned>(value.tm_mday))};
  if (!date.ok() || value.tm_hour > 23 || value.tm_min > 59 || value.tm_sec > 59)
    return {};
  return duration_cast<seconds>((sys_days(date) + hours(value.tm_hour) +
      minutes(value.tm_min) + seconds(value.tm_sec)).time_since_epoch()).count();
}

// SHA-256 over the exact cached bytes: independent of file timestamps, names
// and reload generations. This helper is not a public cryptography API.
inline std::string static_file_etag(std::string_view bytes) {
  constexpr std::uint32_t k[] = {
      0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
      0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
      0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
      0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
      0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
      0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
      0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
      0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
  std::array<std::uint32_t, 8> h{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                                0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
  const std::size_t blocks = bytes.size() / 64 + (bytes.size() % 64 < 56 ? 1 : 2);
  const auto bits = static_cast<std::uint64_t>(bytes.size()) * 8;
  for (std::size_t block = 0; block < blocks; ++block) {
    std::array<std::uint32_t, 64> w{};
    for (std::size_t i = 0; i < 64; ++i) {
      const auto pos = block * 64 + i;
      unsigned char byte = 0;
      if (pos < bytes.size()) byte = static_cast<unsigned char>(bytes[pos]);
      else if (pos == bytes.size()) byte = 0x80;
      else if (pos >= blocks * 64 - 8)
        byte = static_cast<unsigned char>(bits >> ((blocks * 64 - 1 - pos) * 8));
      w[i / 4] |= std::uint32_t(byte) << (24 - (i % 4) * 8);
    }
    for (int i = 16; i < 64; ++i) {
      const auto a = w[i-15], b = w[i-2];
      w[i] = w[i-16] + (std::rotr(a,7)^std::rotr(a,18)^(a>>3)) +
             w[i-7] + (std::rotr(b,17)^std::rotr(b,19)^(b>>10));
    }
    auto [a,b,c,d,e,f,g,hv] = h;
    for (int i = 0; i < 64; ++i) {
      const auto t1 = hv + (std::rotr(e,6)^std::rotr(e,11)^std::rotr(e,25)) +
          ((e&f)^(~e&g)) + k[i] + w[i];
      const auto t2 = (std::rotr(a,2)^std::rotr(a,13)^std::rotr(a,22)) +
          ((a&b)^(a&c)^(b&c));
      hv=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hv;
  }
  std::string result = "\"";
  constexpr char hex[] = "0123456789abcdef";
  for (auto word : h) for (int shift = 28; shift >= 0; shift -= 4)
    result += hex[(word >> shift) & 15];
  return result + '"';
}

inline bool static_file_tag_matches(std::string_view list,
                                    std::string_view tag, bool strong) {
  helpers::ows_trim(list);
  if (list == "*") return true;
  while (!list.empty()) {
    helpers::ows_ltrim(list);
    if (!list.empty() && list.front() == ',') { list.remove_prefix(1); continue; }
    if (list.empty()) break;
    const bool weak = list.starts_with("W/");
    if (weak) list.remove_prefix(2);
    if (list.empty() || list.front() != '"') return false;
    const auto end = list.find('"', 1);
    if (end == list.npos) return false;
    auto candidate = list.substr(0, end + 1);
    auto actual = tag;
    const bool actual_weak = actual.starts_with("W/");
    if (actual_weak) actual.remove_prefix(2);
    if ((!strong || (!weak && !actual_weak)) && candidate == actual) return true;
    list.remove_prefix(end + 1);
    helpers::ows_ltrim(list);
    if (list.empty()) break;
    if (list.front() != ',') return false;
    list.remove_prefix(1);
  }
  return false;
}

inline std::optional<std::size_t> static_file_number(std::string_view text) {
  std::size_t value{};
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
  if (text.empty() || parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
    return {};
  return value;
}

struct static_file_range {
  bool applicable = false;
  bool satisfiable = false;
  std::size_t offset = 0;
  std::size_t length = 0;
};
inline static_file_range static_file_parse_range(std::string_view text, std::size_t size) {
  // Unsupported units and multiple ranges are deliberately ignored.
  if (text.size() < 6 || !helpers::iequals(text.substr(0, 6), "bytes=") ||
      text.find(',') != text.npos) return {};
  text.remove_prefix(6);
  helpers::ows_trim(text);
  const auto dash = text.find('-');
  if (dash == text.npos) return {};
  const auto left = text.substr(0, dash), right = text.substr(dash + 1);
  auto first = static_file_number(left), last = static_file_number(right);
  if (left.empty()) {
    if (!last) return {};
    const auto length = std::min(*last, size);
    return {true, length != 0, size - length, length};
  }
  if (!first || (!right.empty() && !last) || (last && *last < *first)) return {};
  if (*first >= size) return {true, false, 0, 0};
  return {true, true, *first, (last ? std::min(*last, size - 1) : size - 1) - *first + 1};
}
}  // namespace martianlabs::doba::protocol::http::v11::detail
#endif
