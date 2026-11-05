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

#include "tool_json.h"
#include "tool_process.h"
#include "tool_response.h"
#include "tool_sha256.h"
#include "../qualification/command_progress.h"

using namespace martianlabs::doba::tests::tools;

template<class Function>
static void rejected(Function function) {
  bool failed = false;
  try { function(); } catch (const std::exception&) { failed = true; }
  require(failed, "Negative tool control was accepted");
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] memory_stream                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct memory_stream {
  std::string bytes;
  size_t offset{};
  // +-------------------------------------------------------------------------+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  std::string receive(size_t maximum) {
    auto result = bytes.substr(offset, maximum);
    offset += result.size();
    return result;
  }
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  if (argc >= 2 && std::string(argv[1]) == "--echo") {
    json arguments(json::array{});
    for (int index = 2; index < argc; ++index) {
      arguments.items().push_back(argv[index]);
    }
    std::cout << arguments.dump();
    return 0;
  }
  if (argc == 2 && std::string(argv[1]) == "--large") {
    std::cout << std::string(2 * 1024 * 1024, 'o');
    std::cerr << std::string(2 * 1024 * 1024, 'e');
    return 0;
  }
  if (argc == 2 && std::string(argv[1]) == "--stdin") {
    std::string line;
    std::getline(std::cin, line);
    std::cout << line;
    return 0;
  }
  if (argc == 2 && std::string(argv[1]) == "--environment") {
    auto value = environment_value("DOBA_NATIVE_PROCESS_CONTROL");
    std::cout << (value ? *value : "unset");
    return 0;
  }
  if (argc == 2 && std::string(argv[1]) == "--progress") {
    std::cout << "first\n" << std::flush;
    std::this_thread::sleep_for(200ms);
    std::cerr << "diagnostic\n" << std::flush;
    std::cout << "last\n" << std::flush;
    return 0;
  }
  if (argc == 2 && std::string(argv[1]) == "--suspend") {
    std::cout << "before deadline\n" << std::flush;
    std::this_thread::sleep_for(10s);
    return 0;
  }
  if (argc >= 3 && std::string(argv[1]) == "--diagnostic") {
    std::cerr << argv[2];
    return 0;
  }
  return guarded([&] {
    require(process::text("") == "" &&
      process::text("\r\n\n\rX\r\r\nend\r") == "\n\n\rX\r\nend\r",
      "CRLF normalization changed non-CRLF bytes");
    {
      std::string value;
      for (int index = 0; index < 100000; ++index) value += "row\r\n";
      auto normalized = process::text(value);
      require(normalized.size() == 400000 &&
        normalized.find('\r') == std::string::npos,
        "Large CRLF normalization lost bytes");
    }
    {
      temporary_directory directory;
      auto log = directory.path() / "progress.log";
      process child({argv[0], "--progress"});
      command_progress progress(child, "positive progress", log);
      require(progress.wait(5s) == 0, "Progress child failed");
      auto value = process::text(read(log));
      require(value.find("first\n") != std::string::npos &&
        value.find("last\n") != std::string::npos &&
        value.find("diagnostic\n") != std::string::npos,
        "Progress log lost stdout/stderr");
    }
    {
      temporary_directory directory;
      auto log = directory.path() / "deadline.log";
      process child({argv[0], "--suspend"});
      command_progress progress(child, "deadline control", log);
      rejected([&] { progress.wait(200ms); });
      require(child.poll() && read(log).find("deadline exceeded") !=
        std::string::npos, "Progress timeout lost cleanup or diagnostic");
    }
    auto root = fs::path(DOBA_TOOL_SOURCE_ROOT);
    for (auto directory : {"tests", "include", "examples", "benchmarks",
      "cmake", "compliance", ".github"}) {
      for (const auto& entry : fs::recursive_directory_iterator(
        root / directory)) {
        require(entry.path().extension() != ".py",
          "Project tooling must be C++: " + entry.path().string());
      }
    }
    clean_diagnostics("ordinary example diagnostics\n");
    clean_diagnostics("AddressSanitizer Init done\n");
    std::vector<std::string> command{argv[0], "--echo", "", "two words",
      "quote\"value", "trailing\\", "\\\"", "tab\tvalue", "line\nvalue"};
    auto echoed = run(command, 5s);
    require(echoed.code == 0 && echoed.errors.empty(),
      "Quoted process invocation failed");
    auto echoed_values = json::parse(echoed.output);
    require(echoed_values.items().size() == command.size() - 2,
      "Child argument count differs");
    for (size_t index = 2; index < command.size(); ++index) {
      require(echoed_values[index - 2].string() == command[index],
        "Child argument quoting changed bytes");
    }
    auto large = run({argv[0], "--large"}, 5s);
    require(large.code == 0 && large.output == std::string(2 * 1024 * 1024, 'o')
      && large.errors == std::string(2 * 1024 * 1024, 'e'),
      "Child output was truncated or blocked");
    {
      process child({argv[0], "--stdin"});
      child.send("native stdin\n");
      require(child.wait(5s) == 0 && child.output() == "native stdin",
        "Child stdin forwarding failed");
    }
    {
      process child({argv[0], "--environment"},
        {{"DOBA_NATIVE_PROCESS_CONTROL", "private value"}});
      require(child.wait(5s) == 0 && child.output() == "private value",
        "Child environment override failed");
    }
    {
      process child({argv[0], "--environment"}, {},
        {"DOBA_NATIVE_PROCESS_CONTROL"});
      require(child.wait(5s) == 0 && child.output() == "unset",
        "Child environment removal failed");
    }
    for (auto finding : {"ERROR: AddressSanitizer: heap-use-after-free",
      "WARNING: ThreadSanitizer: data race",
      "SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior",
      "ERROR: LeakSanitizer: detected memory leaks",
      "FATAL: ThreadSanitizer: unexpected memory mapping",
      "AddressSanitizer:DEADLYSIGNAL",
      "test.cpp:2: runtime error: index out of bounds",
      "ThreadSanitizer: reported 3 warnings"}) {
      auto child = run({argv[0], "--diagnostic", finding}, 5s);
      require(child.code == 0, "Diagnostic child did not exit cleanly");
      rejected([&] { clean_diagnostics(child.errors); });
    }
    require(sha256("") ==
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" &&
      sha256("abc") ==
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
      "SHA-256 known-answer failure");
    require(sha256(std::string(1000000, 'a')) ==
      "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
      "SHA-256 multiblock known-answer failure");
    for (auto invalid : {"", "01", "[1,]", "{\"x\":1,\"x\":2}",
      "1e", "1.", "NaN", "true x", "1e9999", "\"\\ud800\"",
      "\"\\udc00\"", "\"\\uXY00\"", "\"\\x\"", "{\"x\" 1}"}) {
      rejected([&] { json::parse(invalid); });
    }
    auto value = json::parse(
      "{\"a\":[null,true,false,-1.25e2,\"\\u00e9\\ud83d\\ude80\"]}");
    require(json::parse(value.dump()) == value, "JSON round trip differs");
    rejected([&] { json::parse(std::string(129, '[') + "0" +
      std::string(129, ']')); });
    for (auto invalid : {std::string("\xc0\xaf", 2),
      std::string("\xed\xa0\x80", 3), std::string("\xf4\x90\x80\x80", 4),
      std::string("\xc2", 1), std::string("\xe0\x80\xaf", 3)}) {
      rejected([&] { json::parse('"' + invalid + '"'); });
    }
    const std::string valid =
      "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok";
    memory_stream stream{valid + valid};
    expect_response(stream, 200, "ok"); expect_response(stream, 200, "ok");
    require(stream.receive(1).empty(), "Oracle left unread successor bytes");
    memory_stream binary{"HTTP/1.1 201 \r\ncontent-length: \t0002\t\r\n"
      "X-Info: a\r\nX-Info: b\r\n\r\n" + std::string("\0\xff", 2)};
    auto unique = binary;
    rejected([&] { response(unique, true); });
    expect_response(binary, 201, std::string("\0\xff", 2));
    std::vector<std::string> fields{
      "Content-Length: 999\r\ncontent-length: 2",
      "Content-Length: 2\r\nContent-Length: 2",
      "Content-Length: 2\r\nTransfer-Encoding: chunked", "Content-Length: +2",
      "Content-Length: -1", "Content-Length: 2, 2", "Content-Length: 1048577",
      "Content-Length: 2\r\nX Bad: value",
      "Content-Length: 2\r\nDate: a\r\ndate: b",
      "Content-Length: 2\r\nX: " + std::string(16384, 'x')};
    for (char byte : {'\0', '\v', '\x7f'}) {
      fields.push_back("Content-Length: 2\r\nX: value" +
        std::string(1, byte) + "hidden");
    }
    for (const auto& field : fields) {
      memory_stream invalid{"HTTP/1.1 200 OK\r\n" + field + "\r\n\r\nok"};
      rejected([&] { response(invalid); });
    }
    for (auto bytes : {"HTTP/1.1 099 Invalid\r\nContent-Length: 2\r\n\r\nok",
      "HTTP/1.1 600 Invalid\r\nContent-Length: 2\r\n\r\nok",
      "HTTP/1.1 200 OK\nContent-Length: 2\n\nok",
      "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nok",
      "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n"}) {
      memory_stream invalid{bytes};
      rejected([&] { response(invalid); });
    }
    memory_stream invalid{"HTTP/1.1 200 bad" + std::string(1, '\0') +
      "reason\r\nContent-Length: 2\r\n\r\nok"};
    rejected([&] { response(invalid); });
    std::cout << "Diagnostics, native tooling and independent wire oracle: "
      "PASS\n";
  });
}
