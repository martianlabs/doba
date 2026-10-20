"""Compile and kill three bounded HTTP mutants in independent header copies.

Run on Linux: python3 tests/mutation/run_detection.py --output out/mutations
The original headers are never edited. Tests execute only with Release flags.
"""

import argparse
import difflib
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time


CASES = [
    {
        "id": "body-limit",
        "header": "protocol/http/v11/headers/content_length.h",
        "before": "if (policies.max_request_body_size != 0 &&",
        "after": "if (false && policies.max_request_body_size != 0 &&",
        "test": "detection rejects a body one byte above its configured limit",
        "assertion": "result.code == protocol::deserialization_status::kInvalidSource",
    },
    {
        "id": "response-order",
        "header": "protocol/http/v11/engine_async.h",
        "before": "if (final && position == next_send_.load(std::memory_order_relaxed) &&",
        "after": "if (final && true &&",
        "test": "detection queues a successor response until the first completes",
        "assertion": "(sends) == (0)",
    },
    {
        "id": "rejection-successor",
        "header": "protocol/http/v11/engine.h",
        "before": "if (rejected) query_for_close();",
        # Notify the transport, but omit the engine's terminal-state update.
        # A later receive must still be rejected even before socket retirement.
        "after": "if (rejected) on_close_();",
        "test": "detection never dispatches a successor after terminal rejection",
        "assertion": "(dispatched) == (0)",
    },
]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def execute(command, log, *, env=None, timeout=180):
    with log.open("w") as output:
        output.write("Command: " + json.dumps(command) + "\n")
        output.flush()
        result = subprocess.run(command, stdout=output, stderr=subprocess.STDOUT,
                                env=env, timeout=timeout)
    return result.returncode, log.read_text(errors="replace")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--compiler", default="clang++")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    args.output.mkdir(parents=True, exist_ok=True)
    # A fresh run preserves prior results and never resets a caller's directory.
    run = args.output / time.strftime("%Y%m%d-%H%M%S")
    run.mkdir()
    headers = sorted((root / "include").rglob("*.h"))
    before_hashes = {str(p.relative_to(root)): digest(p) for p in headers}
    selected = root / "tests/unit/protocol/http/v11/regression_detection_tests.cpp"
    test_hash = digest(selected)
    env = os.environ.copy()
    env["ASAN_OPTIONS"] = "halt_on_error=1:detect_leaks=1"
    env["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
    results = []
    try:
        original = run / "original"
        original.mkdir()
        shutil.copytree(root / "include", original / "include")
        (run / "revision.txt").write_text(subprocess.check_output(
            ["git", "-C", str(root), "rev-parse", "HEAD"], text=True))
        (run / "compiler.txt").write_text(subprocess.check_output([args.compiler, "--version"], text=True))
        (run / "source-sha256.json").write_text(json.dumps(
            {**before_hashes, str(selected.relative_to(root)): test_hash}, indent=2))

        def compile_at(directory):
            executable = directory / "detection_tests"
            command = [args.compiler, "-std=c++20", "-O3", "-DNDEBUG", "-g",
                       "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-pthread",
                       "-fsanitize=address,undefined", "-fno-sanitize-recover=undefined",
                       "-fno-omit-frame-pointer", "-I" + str(directory / "include"),
                       "-I" + str(root / "tests/unit/helpers"),
                       str(root / "tests/unit/helpers/test_helper.cpp"), str(selected),
                       "-o", str(executable)]
            code, _ = execute(command, directory / "compile.log", env=env)
            if code != 0:
                raise RuntimeError(f"Compilation failed; this is not a killed mutant: {directory}")
            return executable

        baseline = compile_at(original)
        for case in CASES:
            code, output = execute([str(baseline), "--name", case["test"]],
                                   original / (case["id"] + ".log"), env=env, timeout=15)
            if code != 0 or "running 1 unit tests" not in output:
                raise RuntimeError(f"Original test did not pass exactly once: {case['id']}")
        for case in CASES:
            directory = run / case["id"]
            directory.mkdir()
            shutil.copytree(original / "include", directory / "include")
            header = directory / "include" / case["header"]
            original_text = header.read_text()
            if original_text.count(case["before"]) != 1:
                raise RuntimeError(f"Mutation anchor changed: {case['id']}")
            changed = original_text.replace(case["before"], case["after"], 1)
            header.write_text(changed)
            (directory / "mutation.diff").write_text("".join(difflib.unified_diff(
                original_text.splitlines(keepends=True), changed.splitlines(keepends=True),
                fromfile=case["header"], tofile=case["header"])))
            executable = compile_at(directory)
            code, output = execute([str(executable), "--name", case["test"]],
                                   directory / "test.log", env=env, timeout=15)
            # A compiler error, crash, sanitizer finding or timeout does not
            # demonstrate that the planned functional assertion detected it.
            killed = (code == 1 and case["assertion"] in output and
                      "running 1 unit tests" in output and
                      "AddressSanitizer" not in output and "runtime error:" not in output)
            results.append({"mutation": case["id"], "test": case["test"],
                            "expected_assertion": case["assertion"], "killed": killed})
            (run / "results.json").write_text(json.dumps(results, indent=2))
            if not killed:
                raise RuntimeError(f"Mutation not killed by its planned assertion: {directory}")
            print(f"PASS: {case['id']} killed by {case['assertion']}", flush=True)
    finally:
        assert before_hashes == {str(p.relative_to(root)): digest(p) for p in headers}, "Original headers changed"
        assert test_hash == digest(selected), "Original detector tests changed"
    print(f"All three mutants detected; original headers unchanged. Evidence: {run}")


if __name__ == "__main__":
    main()
