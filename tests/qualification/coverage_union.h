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

#ifndef martianlabs_doba_tests_qualification_coverage_union_h
#define martianlabs_doba_tests_qualification_coverage_union_h

#include "../helpers/tool_json.h"

#include <array>
#include <cstdint>
#include <set>

namespace martianlabs::doba::tests::tools {

// +---------------------------------------------------------------------------+
// | [>] source_coverage                                            ( struct ) |
// +---------------------------------------------------------------------------+
struct source_coverage {
  std::map<unsigned, double> lines;
  std::map<std::array<unsigned, 5>, std::array<bool, 2>> branches;
  bool operator==(const source_coverage&) const = default;
};
using coverage_files = std::map<std::string, source_coverage>;

inline std::string production_name(const fs::path& filename,
                                   const fs::path& canonical_root) {
  auto relative = fs::weakly_canonical(filename).lexically_relative(
    canonical_root).generic_string();
  return relative.starts_with("include/") ? relative : "";
}

inline void merge(coverage_files& files, const json& document,
                  const std::string& lcov, const fs::path& root) {
  auto canonical_root = fs::canonical(root);
  for (const auto& data : document["data"].items()) {
    for (const auto& item : data["files"].items()) {
      auto name = production_name(item["filename"].string(), canonical_root);
      if (name.empty()) continue;
      auto& file = files[name];
      if (!item.contains("branches")) continue;
      for (const auto& branch : item["branches"].items()) {
        require(branch.items().size() >= 9, "Malformed LLVM branch location");
        std::array<unsigned, 5> key{};
        for (size_t index = 0; index < 5; ++index) {
          auto number = branch[index == 4 ? 8 : index].number();
          require(number >= 0 && number <= UINT32_MAX &&
            std::floor(number) == number, "Invalid LLVM source location");
          key[index] = static_cast<unsigned>(number);
        }
        auto& outcomes = file.branches[key];
        outcomes[0] = outcomes[0] || branch[size_t{4}].number() > 0;
        outcomes[1] = outcomes[1] || branch[size_t{5}].number() > 0;
      }
    }
  }
  std::string name;
  for (const auto& line : lines(lcov)) {
    if (line.starts_with("SF:")) {
      name = production_name(line.substr(3), canonical_root);
    } else if (line == "end_of_record") name.clear();
    else if (!name.empty() && line.starts_with("DA:")) {
      auto comma = line.find(',');
      require(comma != std::string::npos, "Invalid LCOV line");
      auto number = std::stoul(line.substr(3, comma - 3));
      require(number > 0 && number <= UINT32_MAX, "Invalid LCOV source line");
      double count = std::stod(line.substr(comma + 1));
      require(std::isfinite(count) && count >= 0, "Invalid LCOV hit count");
      auto& previous = files[name].lines[static_cast<unsigned>(number)];
      previous = std::max(previous, count);
    }
  }
}

inline std::string module(const std::string& name) {
  if (name.starts_with("include/common/")) return "common";
  if (name.starts_with("include/network/")) return "network";
  if (name.starts_with("include/transport/")) {
    return name.find("/tls") != std::string::npos ? "tls" : "tcp";
  }
  if (name.starts_with("include/protocol/http/v11/")) {
    auto part = name.substr(26);
    for (auto prefix : {"body/", "headers/", "decoder"}) {
      if (part.starts_with(prefix)) return "http-parser";
    }
    for (auto prefix : {"engine", "request_processor", "pending_response",
                         "connection", "context"}) {
      if (part.starts_with(prefix)) return "http-engine";
    }
    if (part.starts_with("static_file")) return "static-files";
    return "http-messages";
  }
  return name.find("/http/") != std::string::npos ?
    "http-common" : "public-contracts";
}

inline json summarize(const coverage_files& files, const fs::path& root) {
  require(!files.empty(), "No production coverage found");
  json groups(json::object{}), uncovered(json::array{});
  json measured(json::array{}), unmapped(json::array{});
  for (const auto& [name, file] : files) {
    measured.items().push_back(name);
    auto key = module(name);
    if (!groups.contains(key)) {
      groups[key] = json::object{{"files", json::array{}},
        {"lines", json::object{{"count", 0}, {"covered", 0}}},
        {"branches", json::object{{"count", 0}, {"covered", 0}}}};
    }
    auto& group = groups[key];
    group["files"].items().push_back(name);
    auto add = [&](const std::string& metric, size_t count, size_t covered) {
      auto& value = group[metric];
      value["count"] = value["count"].number() + static_cast<double>(count);
      value["covered"] = value["covered"].number() +
        static_cast<double>(covered);
    };
    size_t hit = 0;
    for (const auto& [line, count] : file.lines) {
      (void)line;
      hit += count > 0;
    }
    add("lines", file.lines.size(), hit);
    hit = 0;
    for (const auto& [position, outcomes] : file.branches) {
      hit += outcomes[0] + outcomes[1];
      if (!outcomes[0] || !outcomes[1]) {
        uncovered.items().push_back(json::object{{"file", name},
          {"line", static_cast<int>(position[0])},
          {"column", static_cast<int>(position[1])},
          {"true", outcomes[0]}, {"false", outcomes[1]}});
      }
    }
    add("branches", 2 * file.branches.size(), hit);
  }
  for (auto& [name, group] : groups.members()) {
    (void)name;
    for (auto key : {"lines", "branches"}) {
      auto& counts = group[key];
      double count = counts["count"].number();
      counts["percent"] = count ?
        json(100 * counts["covered"].number() / count) : json();
    }
  }
  std::set<std::string> headers;
  for (const auto& entry : fs::recursive_directory_iterator(root / "include")) {
    if (entry.is_regular_file() && entry.path().extension() == ".h") {
      headers.insert(entry.path().lexically_relative(root).generic_string());
    }
  }
  for (const auto& header : headers) {
    if (!files.contains(header)) unmapped.items().push_back(header);
  }
  return json::object{{"metric",
    "union of executable source lines and condition outcomes"},
    {"modules", groups}, {"measured_headers", measured},
    {"unmapped_headers", unmapped}, {"uncovered_branch_outcomes", uncovered}};
}

inline void enforce(const json& summary, const json& baseline) {
  auto& groups = summary["modules"];
  for (const auto& [name, current] : groups.members()) {
    (void)current;
    require(baseline["modules"].contains(name),
      "Unreviewed measured module: " + name);
  }
  for (const auto& [name, minimum] : baseline["modules"].members()) {
    require(groups.contains(name), "Missing measured module: " + name);
    for (auto metric : {"lines", "branches"}) {
      const auto& floor = minimum[metric];
      const auto& current = groups[name][metric];
      if (floor.null()) {
        require(current["count"].number() == 0,
          "Missing coverage floor: " + name + '/' + metric);
      } else {
        require(floor.numeric() && std::isfinite(floor.number()) &&
          floor.number() >= 0 && floor.number() <= 100,
          "Invalid coverage floor: " + name + '/' + metric);
        require(!current["percent"].null() &&
          current["percent"].number() + 1e-9 >= floor.number(),
          "Coverage regression: " + name + '/' + metric);
      }
    }
  }
  for (auto key : {"measured_headers", "unmapped_headers"}) {
    bool measured = std::string_view(key) == "measured_headers";
    auto& wanted = measured ? baseline[key] : summary[key];
    auto& actual = measured ? summary[key] : baseline[key];
    for (const auto& file : wanted.items()) {
      require(std::find(actual.items().begin(), actual.items().end(), file) !=
        actual.items().end(), (measured ? "Lost measured header: " :
        "New unmapped header: ") + file.string());
    }
  }
}

inline std::string escape_html(std::string_view text) {
  std::string output;
  for (char character : text) {
    switch (character) {
      case '&': output += "&amp;"; break;
      case '<': output += "&lt;"; break;
      case '>': output += "&gt;"; break;
      case '"': output += "&quot;"; break;
      case '\'': output += "&#x27;"; break;
      default: output += character;
    }
  }
  return output;
}

inline std::string markdown_report(const json& summary) {
  std::ostringstream output;
  output << "# Release source coverage\n\n"
    "| Module | Lines | Condition outcomes |\n| --- | --- | --- |\n";
  for (const auto& [name, group] : summary["modules"].members()) {
    output << "| " << name;
    for (auto key : {"lines", "branches"}) {
      const auto& metric = group[key];
      output << " | " << metric["covered"].number() << '/' <<
        metric["count"].number() << " (";
      if (metric["percent"].null()) output << "not applicable";
      else output << std::fixed << std::setprecision(2) <<
        metric["percent"].number() << '%' << std::defaultfloat;
      output << ')';
    }
    output << " |\n";
  }
  output << "\nUnmapped headers: " <<
    summary["unmapped_headers"].items().size() << ".\n"
    "See summary.json for the inventory and uncovered outcomes.\n"
    "Download the coverage artifact for html/index.html.\n"
    "Coverage is not a security certification.\n";
  return output.str();
}

inline void html_report(const coverage_files& files, const json& summary,
                        const fs::path& root, const fs::path& output) {
  require(fs::create_directory(output), "Fresh HTML directory required");
  const std::string style = "<style>body{font:16px system-ui;margin:2rem}"
    "pre{font:13px monospace;line-height:1.5}.miss{background:#ffd6d6}"
    ".hit{background:#d5f5df}td,th{padding:.4rem;text-align:left}</style>";
  std::string index = "<!doctype html><meta charset=\"utf-8\">" + style +
    "<title>Doba Release coverage</title><h1>Release coverage</h1><pre>" +
    escape_html(markdown_report(summary)) + "</pre><ul>";
  size_t number = 0;
  for (const auto& [name, file] : files) {
    auto target = "file-" + std::to_string(number++) + ".html";
    index += "<li><a href=\"" + target + "\">" +
      escape_html(name) + "</a></li>";
    std::string page = "<!doctype html><meta charset=\"utf-8\">" + style +
      "<title>" + escape_html(name) + "</title><h1>" + escape_html(name) +
      "</h1><pre>";
    unsigned line = 0;
    for (const auto& text : lines(read(root / name))) {
      ++line;
      auto found = file.lines.find(line);
      std::string css = found == file.lines.end() ? "" :
        found->second > 0 ? "hit" : "miss";
      page += "<span id=\"L" + std::to_string(line) + "\" class=\"" + css +
        "\">" + std::to_string(line) + ' ' + escape_html(text) + "</span>\n";
    }
    page += "</pre><h2>Condition outcomes</h2><ul>";
    for (const auto& [position, outcomes] : file.branches) {
      auto line_text = std::to_string(position[0]);
      page += "<li><a href=\"#L" + line_text + "\">" + line_text + ':' +
        std::to_string(position[1]) + "</a> true=" +
        (outcomes[0] ? "true" : "false") + ", false=" +
        (outcomes[1] ? "true" : "false") + "</li>";
    }
    write(output / target, page + "</ul>");
  }
  write(output / "index.html", index + "</ul>");
}

} // namespace martianlabs::doba::tests::tools

#endif
