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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

namespace {
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
using corpus = std::map<std::string, std::string>;
// +---------------------------------------------------------------------------+
// | [>] write                                                    ( function ) |
// +---------------------------------------------------------------------------+
void write(const std::filesystem::path& root, const char* harness,
           const corpus& seeds) {
  const auto directory = root / harness;
  std::filesystem::create_directories(directory);
  for (const auto& [name, wire] : seeds) {
    std::ofstream file(directory / ("seed-" + name),
                       std::ios::binary | std::ios::trunc);
    file.write(wire.data(), static_cast<std::streamsize>(wire.size()));
    file.close();
    if (!file) throw std::runtime_error("Cannot write corpus seed: " + name);
  }
  std::cout << harness << ": " << seeds.size() << " canonical seeds in "
            << directory << '\n';
}
}  // namespace

// +---------------------------------------------------------------------------+
// | [>] main                                                     ( function ) |
// +---------------------------------------------------------------------------+
int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: doba_fuzz_seed_corpus <destination>\n";
    return 2;
  }
  try {
    const std::string successor =
        "GET /next HTTP/1.1\r\nHost: example.com\r\n\r\n";
    const corpus chunks{
        {"empty", "0\r\n\r\n"},
        {"payload", "4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n"},
        {"extensions-trailers",
         "3;name=\"a\\\"b\"\r\nabc\r\n0\r\nX-Checksum: ok\r\n\r\n"},
        {"pipeline", "1\r\na\r\n0\r\n\r\n" + successor},
        {"incomplete", "10\r\nabc"},
        {"bad-size", "z\r\nx\r\n0\r\n\r\n"},
        {"overflow", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\r\n"},
        {"bad-data-crlf", "1\r\naX\n0\r\n\r\n"},
        {"forbidden-trailer", "0\r\nContent-Length: 4\r\n\r\n"},
        {"bad-trailer", "0\r\nBad Name: value\r\n\r\n"},
        {"extension-limit",
         "1;x=" + std::string(1100, 'a') + "\r\na\r\n0\r\n\r\n"},
        {"trailer-limit", "0\r\nX: " + std::string(4200, 'a') + "\r\n\r\n"},
        {"payload-limit",
         "1001\r\n" + std::string(4097, 'a') + "\r\n0\r\n\r\n"}};
    std::string query = "GET /?x=1";
    for (int i = 1; i < 33; ++i) query += "&x=1";
    query += " HTTP/1.1\r\nHost: x\r\n\r\n";
    corpus requests{
        {"head-limit-incomplete", "GET / HTTP/1.1\r\nHost: x\r\nX: " + std::string(8192, 'a')},
        {"head-limit-complete", "GET / HTTP/1.1\r\nHost: x\r\nX: " + std::string(8192, 'a') + "\r\n\r\n" + successor},
        {"head-boundary-delimiter", "GET / HTTP/1.1\r\nHost: x\r\nX: " + std::string(4064, 'a') + "\r\n\r\n"},
        {"get", successor},
        {"pipeline", successor + successor},
        {"query",
         "GET /a?x=1&y=two%20words HTTP/1.1\r\nHost: example.com:80\r\n\r\n"},
        {"content-length",
         "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 4\r\n\r\nbody" +
             successor},
        {"storage",
         "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 128\r\n\r\n" +
             std::string(128, 'a') + successor},
        {"expect",
         "POST / HTTP/1.1\r\nHost: x\r\nExpect: "
         "100-continue\r\nContent-Length: 1\r\n\r\nx"},
        {"duplicate-length",
         "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 1\r\nContent-Length: "
         "2\r\n\r\nx" +
             successor},
        {"te-cl",
         "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: "
         "1\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n" +
             successor},
        {"nul",
         std::string("GET / HTTP/1.1\r\nHost: x\r\nX: a") + '\0' + "b\r\n\r\n"},
        {"missing-host", "GET / HTTP/1.1\r\n\r\n"},
        {"connect-missing-port", "CONNECT missing-port HTTP/1.1\r\n\r\n"},
        {"connect-fragmentation-regression",
         "CONNECT T- / HT\x94P" + std::string(10, '\xff') +
             "GETGET HPOS-EnT TTP/1.1\r\n\r\n\r\n"
             "0000tn\xfe\xff\xff\xfe: c1T- " +
             std::string(5, '\0') + '\x02'},
        {"bare-lf", "GET / HTTP/1.1\nHost: x\n\n"},
        {"incomplete",
         "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 9\r\n\r\nabc"},
        {"head-limit", "GET / HTTP/1.1\r\nHost: x\r\nX: " +
                           std::string(4200, 'a') + "\r\n\r\n"},
        {"query-limit", query}};
    for (const auto& [name, wire] : chunks)
      requests.emplace(
          "chunked-" + name,
          "POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n" +
              wire);
    write(argv[1], "decoder", requests);
    write(argv[1], "chunked", chunks);
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
