#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <span>
#include <string>
#include <string_view>

namespace doba_fuzz {
inline void check(bool condition, const char* property) {
  // assert() is disabled in Release. Properties must remain executable.
  if (!condition) {
    std::fprintf(stderr, "doba fuzz property failed: %s\n", property);
    std::abort();
  }
}
inline auto bytes(std::string_view wire) {
  return std::as_bytes(std::span(wire.data(), wire.size()));
}
inline std::size_t fragment_size(std::string_view wire, std::size_t pos) {
  return std::min(wire.size() - pos,
                  1 + static_cast<unsigned char>(wire[pos]) % 31UL);
}
inline std::string chunked_wire(std::string_view payload) {
  if (payload.empty()) return "0\r\n\r\n";
  std::array<char, 32> size_line{};
  std::snprintf(size_line.data(), size_line.size(), "%zx", payload.size());
  return std::string(size_line.data()) + "\r\n" + std::string(payload) + "\r\n0\r\n\r\n";
}
}  // namespace doba_fuzz
