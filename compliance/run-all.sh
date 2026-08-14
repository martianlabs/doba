#!/usr/bin/env bash
set -uo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
doba_host="${DOBA_HOST:-host.docker.internal}"
doba_port="${DOBA_PORT:-8080}"
timestamp="$(date -u +%Y%m%dT%H%M%SZ)"
output_directory="$script_dir/out/all-$timestamp"
summary_path="$output_directory/summary.txt"
h1spec_log="$output_directory/h1spec.log"
http11probe_log="$output_directory/http11probe.log"
report_path="$script_dir/http11probe/out/http11probe/results.json"

mkdir -p "$output_directory"

run_suite() {
  local name="$1"
  local script="$2"
  local log="$3"

  printf '\n=== %s ===\n' "$name"
  DOBA_HOST="$doba_host" DOBA_PORT="$doba_port" \
    bash "$script" 2>&1 | tee "$log"
  return "${PIPESTATUS[0]}"
}

write_summary() {
  printf '%s\n' "$1" | tee -a "$summary_path"
}

summary_value() {
  grep -m 1 -E "^[[:space:]]*\"$1\"[[:space:]]*:" "$report_path" |
    sed -E 's/.*:[[:space:]]*([0-9.]+).*/\1/'
}

verdict_count() {
  awk -v verdict="$1" '
    /^[[:space:]]*"verdict":/ {
      line = $0
      sub(/^[^:]*:[[:space:]]*"/, "", line)
      sub(/"[[:space:]]*,?[[:space:]]*$/, "", line)
      if (line == verdict) count++
    }
    END { print count + 0 }
  ' "$report_path"
}

scored_verdict_count() {
  awk -v wanted_scored="$1" -v wanted_verdict="$2" '
    /^[[:space:]]*"scored":/ {
      scored = $0
      sub(/^[^:]*:[[:space:]]*/, "", scored)
      sub(/,[[:space:]]*$/, "", scored)
    }
    /^[[:space:]]*"verdict":/ {
      verdict = $0
      sub(/^[^:]*:[[:space:]]*"/, "", verdict)
      sub(/"[[:space:]]*,?[[:space:]]*$/, "", verdict)
      if (scored == wanted_scored && verdict == wanted_verdict) count++
    }
    END { print count + 0 }
  ' "$report_path"
}

write_failures() {
  awk '
    function value(line) {
      sub(/^[^:]*:[[:space:]]*/, "", line)
      sub(/,[[:space:]]*$/, "", line)
      sub(/^"/, "", line)
      sub(/"$/, "", line)
      gsub(/\\\\u00A7/, "S", line)
      gsub(/\\\\u2014/, "-", line)
      gsub(/\\\\u002B/, "+", line)
      return line
    }
    /^[[:space:]]*"id":/ {
      id = value($0)
      description = ""
      reference = ""
      expected = ""
      scored = ""
      verdict = ""
      status = ""
      connection = ""
    }
    /^[[:space:]]*"description":/ { description = value($0) }
    /^[[:space:]]*"rfcReference":/ { reference = value($0) }
    /^[[:space:]]*"expected":/ { expected = value($0) }
    /^[[:space:]]*"scored":/ { scored = value($0) }
    /^[[:space:]]*"verdict":/ { verdict = value($0) }
    /^[[:space:]]*"statusCode":/ { status = value($0) }
    /^[[:space:]]*"connectionState":/ { connection = value($0) }
    /^[[:space:]]*}[,]?$/ {
      if (id != "" && verdict == "Fail") {
        if (scored == "true") {
          print "  " id (reference == "" ? "" : " [" reference "]") " - " description
        } else {
          print "  unscored " id " - " description
        }
        print "    expected: " expected "; received: " status " / " connection
      }
      id = ""
    }
  ' "$report_path" | tee -a "$summary_path"
}

h1spec_exit_code=0
http11probe_exit_code=0
run_suite "h1spec" "$script_dir/run-h1spec.sh" "$h1spec_log" || h1spec_exit_code=$?
run_suite "Http11Probe" "$script_dir/run-http11probe.sh" "$http11probe_log" || http11probe_exit_code=$?

printf '\n'
write_summary "Compliance execution summary"
write_summary "Target: $doba_host:$doba_port"
h1spec_result="$(grep -Eo '[0-9]+ out of [0-9]+ tests passed\.' "$h1spec_log" | tail -n 1 || true)"
h1spec_total=""
h1spec_passed=""
h1spec_failed=""
if [[ "$h1spec_result" =~ ^([0-9]+)\ out\ of\ ([0-9]+) ]]; then
  h1spec_passed="${BASH_REMATCH[1]}"
  h1spec_total="${BASH_REMATCH[2]}"
  h1spec_failed=$((h1spec_total - h1spec_passed))
  write_summary ""
  write_summary "h1spec"
  write_summary "  Total: $h1spec_total | Passed: $h1spec_passed | Failed: $h1spec_failed | Warnings: 0"
  write_summary "  Runner exit code: $h1spec_exit_code"
else
  write_summary ""
  write_summary "h1spec"
  write_summary "  No complete test result (runner exit code: $h1spec_exit_code)"
fi

overall_exit_code=0
if [[ "$h1spec_exit_code" -ne 0 || "$http11probe_exit_code" -ne 0 ]]; then
  overall_exit_code=1
fi

http11probe_total=""
if [[ "$http11probe_exit_code" -eq 0 && -f "$report_path" ]]; then
  http11probe_total="$(summary_value total)"
  http11probe_passed="$(verdict_count Pass)"
  http11probe_failed="$(verdict_count Fail)"
  http11probe_warnings="$(verdict_count Warn)"
  http11probe_errors="$(summary_value errors)"
  scored_passed="$(scored_verdict_count true Pass)"
  scored_failed="$(scored_verdict_count true Fail)"
  scored_warnings="$(scored_verdict_count true Warn)"
  unscored_passed="$(scored_verdict_count false Pass)"
  unscored_failed="$(scored_verdict_count false Fail)"
  unscored_warnings="$(scored_verdict_count false Warn)"
  duration="$(summary_value durationMs)"
  write_summary ""
  write_summary "Http11Probe"
  write_summary "  Total: $http11probe_total | Passed: $http11probe_passed | Failed: $http11probe_failed | Warnings: $http11probe_warnings | Errors: $http11probe_errors"
  write_summary "  Scored: Passed: $scored_passed | Failed: $scored_failed | Warnings: $scored_warnings"
  write_summary "  Unscored: Passed: $unscored_passed | Failed: $unscored_failed | Warnings: $unscored_warnings"
  write_summary "  Duration: $duration ms | Runner exit code: $http11probe_exit_code"
  write_summary "  Failures:"
  write_failures

  if [[ "$scored_failed" -ne 0 || "$http11probe_errors" -ne 0 ]]; then
    overall_exit_code=1
  fi
else
  write_summary ""
  write_summary "Http11Probe"
  write_summary "  No complete test result (runner exit code: $http11probe_exit_code)"
fi

if [[ -n "$h1spec_total" && -n "$http11probe_total" ]]; then
  global_total=$((h1spec_total + http11probe_total))
  global_passed=$((h1spec_passed + http11probe_passed))
  global_failed=$((h1spec_failed + http11probe_failed))
  write_summary ""
  write_summary "Global"
  write_summary "  Tests executed: $global_total"
  write_summary "  Passed: $global_passed | Failed: $global_failed | Warnings: $http11probe_warnings | Errors: $http11probe_errors"
fi

write_summary ""
write_summary "Logs: $output_directory"
if [[ -f "$report_path" ]]; then
  write_summary "Http11Probe report: $report_path"
fi

if [[ "$overall_exit_code" -eq 0 ]]; then
  write_summary "Runner status: completed without scored failures or errors"
else
  write_summary "Runner status: completed with test failures or runner errors"
fi

exit "$overall_exit_code"
