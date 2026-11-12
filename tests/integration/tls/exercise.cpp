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

#include <future>

#include "../../helpers/tool_response.h"
#include "fixture_process.h"

using namespace martianlabs::doba::tests::tools;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  return guarded([&] {
    require(argc == 4, "Expected TLS fixture, certificate and key");
    const std::string binary = argv[1], certificate = argv[2], key = argv[3];
    auto connect = [&](uint16_t port, const std::string& alpn = "doba-echo",
                       int version = 0, bool fragmented = false) {
      auto result = std::make_unique<peer>(port);
      result->tls(certificate, version, alpn, fragmented);
      return result;
    };
    auto rejected = [&](auto function) {
      bool failed = false;
      try {
        function();
      } catch (const std::exception&) {
        failed = true;
      }
      require(failed, "TLS rejection control was accepted");
    };
    auto exchange = [&](uint16_t port, int index) {
      auto connection = connect(port);
      auto text = std::to_string(index);
      connection->send(text + '\n');
      connection->expect("echo:" + text + '\n');
    };
    {
      fixture_process server(binary, certificate, key);
      auto port = server.port();
      for (int version : {TLS1_2_VERSION, TLS1_3_VERSION}) {
        auto connection = connect(port, "doba-echo", version);
        require(connection->alpn() == "doba-echo", "Echo ALPN mismatch");
        std::string input, output;
        for (int index = 0; index < 16; ++index) {
          input += "a\nb\n";
          output += "echo:a\necho:b\n";
        }
        connection->send(input);
        connection->expect(output);
        for (const auto& [command, expected] :
             std::vector<std::pair<std::string, std::string>>{
                 {"source", "pre:body:tail"},
                 {"empty", "pre:body:"},
                 {"large", "pre:" + std::string(128 * 1024, 's')},
                 {"source-close", "pre:body:tail"}}) {
          connection->send(command + '\n');
          connection->expect(expected);
        }
        require(connection->receive(1).empty(), "Source close failed");
        connection->shutdown_tls();
      }
      {
        auto connection = connect(port);
        connection->send(std::string(33000, 'x') + '\n');
        connection->expect("echo:" + std::string(33000, 'x') + '\n');
        connection->send("released\n");
        connection->expect("tail");
        connection->send("after\n");
        connection->expect("echo:after\n");
        connection->shutdown_tls();
      }
      {
        auto connection = connect(port);
        connection->send("empty-close\n");
        connection->expect("pre:body:");
        require(connection->receive(1).empty(), "Empty source close failed");
        connection->shutdown_tls();
      }
      {
        auto connection = connect(port);
        connection->send("second-source\n");
        require(connection->receive(1).empty(), "Second source accepted");
      }
      {
        auto connection = connect(port, "doba-echo", 0, true);
        connection->send("fragmented\n");
        connection->expect("echo:fragmented\n");
      }
      rejected([&] {
        peer connection(port);
        connection.tls({}, 0, "", false);
      });
      rejected([&] {
        peer connection(port);
        connection.tls(certificate, 0, "", false, "wrong.invalid");
      });
      for (int batch = 0; batch < 4; ++batch) {
        std::vector<std::future<void>> exchanges;
        for (int index = 0; index < 8; ++index) {
          exchanges.push_back(std::async(std::launch::async, [&, index, batch] {
            exchange(port, batch * 8 + index);
          }));
        }
        for (auto& future : exchanges) future.get();
      }
      for (auto token : {"h2", "unrelated"}) {
        rejected([&] { connect(port, token); });
      }
      {
        peer bad(port);
        bad.send("not TLS\n");
        require(bad.receive(1).empty(), "Plaintext was accepted by TLS");
      }
      for (auto command : {"bad\n", "failed\n"}) {
        auto bad = connect(port);
        bad->send(command);
        require(bad->receive(1).empty(), "Invalid engine batch accepted");
      }
      exchange(port, 99);
      {
        peer truncated(port);
        truncated.send(std::string("\x16\x03", 2));
      }
      exchange(port, 100);
      {
        std::vector<std::unique_ptr<peer>> pending;
        for (int index = 0; index < 4; ++index) {
          pending.push_back(std::make_unique<peer>(port));
          pending.back()->send(std::string("\x16\x03\x03\x40\0\x01", 6));
        }
        exchange(port, 101);
      }
      exchange(port, 102);
      server.stop();
    }
    std::cout << "TLS versions, pipeline, source, close, fragmentation, "
                 "verification and isolation: PASS\n";
    for (const auto encrypted_rx : {1U, 17U}) {
      fixture_process server(binary, certificate, key, "echo", 4096, 127, 65536,
                             encrypted_rx, 3);
      for (const auto version : {TLS1_2_VERSION, TLS1_3_VERSION}) {
        auto connection = connect(server.port(), "doba-echo", version);
        const std::string text(257, 'r');
        connection->send(text + '\n');
        connection->expect("echo:" + text + '\n');
        connection->send("source-close\n");
        connection->expect("pre:body:tail");
        require(connection->receive(1).empty(), "Small RX lost TLS close");
      }
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key, "echo", 4096, 127);
      for (const auto& [command, expected] :
           std::vector<std::pair<std::string, std::string>>{
               {"exact", std::string(4096, 'e')},
               {"ok", "echo:ok\n"},
               {"inline-exact", "pre:" + std::string(4092, 'e')}}) {
        auto connection = connect(server.port());
        connection->send(command + '\n');
        connection->expect(expected);
      }
      for (auto command : {"over", "inline-over", "sum-over"}) {
        auto connection = connect(server.port());
        connection->send(std::string(command) + '\n');
        require(connection->receive(1).empty(), "Plaintext budget ignored");
      }
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key, "echo", 1048576, 17408,
                             65536);
      auto connection = connect(server.port());
      connection->send("large\n");
      require(connection->receive(1).empty(), "Ciphertext budget ignored");
      exchange(server.port(), 103);
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key, "admission");
      {
        peer first(server.port());
        first.send(std::string("\x16\x03", 2));
        server.child().expect_output("ADMITTED\n");
        rejected([&] { connect(server.port()); });
      }
      std::unique_ptr<peer> recovered;
      auto deadline = clock::now() + 5s;
      while (!recovered) {
        try {
          recovered = connect(server.port());
        } catch (const std::exception&) {
          require(clock::now() < deadline, "TLS admission did not recover");
          std::this_thread::sleep_for(10ms);
        }
      }
      recovered->send("recovered\n");
      recovered->expect("echo:recovered\n");
      recovered.reset();
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key);
      exchange(server.port(), 104);
      server.child().send("restart\n");
      auto deadline = clock::now() + 10s;
      for (;;) {
        auto output = server.child().output();
        if (output.find("READY\n", 6) != std::string::npos) break;
        require(!server.child().poll() && clock::now() < deadline,
                "TLS fixture did not restart");
        std::this_thread::sleep_for(1ms);
      }
      exchange(server.port(), 105);
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key, "echo", 5 * 1024 * 1024,
                             17408, 6 * 1024 * 1024);
      {
        auto connection = connect(server.port());
        connection->receive_buffer(65536);
        connection->send("slow\n");
        std::this_thread::sleep_for(100ms);
        connection->expect("pre:" + std::string(4 * 1024 * 1024, 's'));
        connection->send("after\n");
        connection->expect("echo:after\n");
      }
      {
        auto connection = connect(server.port());
        connection->send("slow\n");
      }
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key, "echo", 1048576, 17408,
                             1);
      rejected([&] { connect(server.port()); });
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key, "http");
      {
        auto connection = connect(server.port(), "http/1.1");
        std::string pipeline;
        for (int index = 0; index < 16; ++index) pipeline += request("/hello");
        connection->send(pipeline);
        for (int index = 0; index < 16; ++index) {
          expect_response(*connection, 200, "hello");
        }
      }
      auto connection = connect(server.port(), "");
      require(connection->alpn().empty(), "Unexpected ALPN without offer");
      connection->send(request("/hello"));
      expect_response(*connection, 200, "hello");
      connection.reset();
      server.stop();
    }
    {
      fixture_process server(binary, certificate, key, "http-auth");
      auto connection = connect(server.port(), "http/1.1");
      require(connection->alpn() == "http/1.1", "Auth ALPN mismatch");
      connection->send(request("/private"));
      require(response(*connection).status == 401, "Missing auth challenge");
      connection->send(
          request("/private", "Authorization: Basic YWxpY2U6c2VjcmV0\r\n"));
      expect_response(*connection, 200, "private");
      connection.reset();
      server.stop();
    }
    auto invalid = [&](const std::vector<std::string>& arguments,
                       const std::string& message) {
      auto command = std::vector<std::string>{binary};
      command.insert(command.end(), arguments.begin(), arguments.end());
      auto result = run(command, 5s);
      clean_diagnostics(result.errors);
      require(
          result.code == 1 && result.errors.find(message) != std::string::npos,
          "TLS configuration rejection mismatch");
    };
    for (auto budgets : std::vector<std::vector<std::string>>{
             {"0", "1", "1"}, {"1", "0", "1"}, {"1", "1", "0"}}) {
      std::vector<std::string> values{certificate, key, "8443", "echo"};
      values.insert(values.end(), budgets.begin(), budgets.end());
      invalid(values, "Invalid TLS policies");
    }
    invalid({certificate + ".missing", key, "8443", "echo"},
            "Cannot configure TLS certificate or key");
    invalid({certificate, key, "8443", "http-invalid"}, "ALPN");
    std::cout << "TLS budgets, admission, restart, slow reader, HTTPS and "
                 "invalid configuration: PASS\n";
  });
}
