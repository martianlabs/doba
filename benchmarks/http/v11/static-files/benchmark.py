#                              _       _
#                           __| | ___ | |__   __ _
#                          / _` |/ _ \| '_ \ / _` |
#                         | (_| | (_) | |_) | (_| |
#                          \__,_|\___/|_.__/ \__,_|
#
#                              Apache License
#                        Version 2.0, January 2004
#                     http://www.apache.org/licenses/
#
# Copyright 2026 martianLabs
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

"""Compare direct and cached static delivery through the unchanged TCP transport.

Python 3 standard library only. Application-cold does not mean OS page-cache cold.
Run against a Release build of the static_file_server example on an idle host.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from contextlib import closing
import ctypes
import http.client
import json
import os
from pathlib import Path
import socket
import subprocess
import tempfile
import threading
import time


def metrics(process):
    if os.name == "nt":
        from ctypes import wintypes
        handle = wintypes.HANDLE(int(process._handle))
        kernel = ctypes.WinDLL("kernel32", use_last_error=True)
        values = [wintypes.FILETIME() for _ in range(4)]
        if not kernel.GetProcessTimes(handle, *(ctypes.byref(v) for v in values)):
            raise ctypes.WinError(ctypes.get_last_error())
        def ticks(value):
            return (value.dwHighDateTime << 32) | value.dwLowDateTime

        cpu = sum(ticks(v) for v in values[2:]) / 10_000_000
        count = wintypes.DWORD()
        kernel.GetProcessHandleCount(handle, ctypes.byref(count))
        class Counters(ctypes.Structure):
            _fields_ = [("cb", wintypes.DWORD), ("faults", wintypes.DWORD)] + [
                (name, ctypes.c_size_t) for name in (
                    "peak_rss", "rss", "peak_paged", "paged", "peak_nonpaged",
                    "nonpaged", "pagefile", "peak_pagefile")]
        memory = Counters()
        memory.cb = ctypes.sizeof(memory)
        if not ctypes.WinDLL("psapi").GetProcessMemoryInfo(
            handle, ctypes.byref(memory), memory.cb
        ):
            raise ctypes.WinError(ctypes.get_last_error())
        return {"cpu_seconds": cpu, "rss_bytes": memory.rss,
                "peak_rss_bytes": memory.peak_rss, "handles": count.value}
    proc = Path(f"/proc/{process.pid}")
    stat = (proc / "stat").read_text().split(")", 1)[1].split()
    status = dict(
        line.split(":", 1)
        for line in (proc / "status").read_text().splitlines()
    )
    return {"cpu_seconds": (int(stat[11]) + int(stat[12])) / os.sysconf("SC_CLK_TCK"),
            "rss_bytes": int(status["VmRSS"].split()[0]) * 1024,
            "peak_rss_bytes": int(status["VmHWM"].split()[0]) * 1024,
            "handles": len(list((proc / "fd").iterdir()))}


def measure(binary, root, size, files, clients, requests, direct, delay):
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
    command = (
        [str(binary), str(root)]
        + (["--no-cache"] if direct else [])
        + [str(port)]
    )
    process = subprocess.Popen(
        command, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE
    )
    try:
        deadline = time.monotonic() + 10
        while True:
            if process.poll() is not None:
                raise RuntimeError(process.stderr.read().decode(errors="replace"))
            try:
                with socket.create_connection(("127.0.0.1", port), timeout=.2):
                    break
            except OSError:
                if time.monotonic() > deadline:
                    raise RuntimeError("Server did not start")
                time.sleep(.02)
        def fetch(connection, index):
            start = time.perf_counter()
            connection.request("GET", f"/assets/{index % files}.bin")
            response = connection.getresponse()
            body = bytearray()
            while chunk := response.read(8192):
                body.extend(chunk)
                if delay:
                    time.sleep(delay / 1000)
            if response.status != 200 or body != b"x" * size:
                raise RuntimeError("Incorrect static response")
            return (time.perf_counter() - start) * 1000
        with closing(
            http.client.HTTPConnection("127.0.0.1", port, timeout=30)
        ) as connection:
            cold = fetch(connection, 0)
            for i in range(files):
                fetch(connection, i)
        before = metrics(process)
        barrier = threading.Barrier(clients)
        def worker(worker_id):
            with closing(
                http.client.HTTPConnection("127.0.0.1", port, timeout=30)
            ) as connection:
                barrier.wait(timeout=30)
                return [
                    fetch(connection, i)
                    for i in range(worker_id, requests, clients)
                ]
        start = time.perf_counter()
        with ThreadPoolExecutor(clients) as executor:
            results = list(executor.map(worker, range(clients)))
        elapsed = time.perf_counter() - start
        after = metrics(process)
        samples = sorted(value for group in results for value in group)
        def percentile(fraction):
            index = min(
                len(samples) - 1, int((len(samples) - 1) * fraction)
            )
            return samples[index]

        return {"mode": "direct" if direct else "cached", "bytes": size,
                "files": files, "clients": clients, "requests": requests,
                "read_delay_ms": delay, "application_cold_ms": cold,
                "requests_per_second": requests / elapsed,
                "p50_ms": percentile(.50),
                "p95_ms": percentile(.95),
                "p99_ms": percentile(.99),
                "server_cpu_seconds": after["cpu_seconds"] - before["cpu_seconds"],
                "server_rss_bytes": after["rss_bytes"],
                "server_peak_rss_bytes": after["peak_rss_bytes"],
                "server_handles_after": after["handles"]}
    finally:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        process.stderr.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument(
        "--sizes", type=int, nargs="+", default=[64, 4096, 262144, 2097152]
    )
    parser.add_argument("--files", type=int, nargs="+", default=[1, 32])
    parser.add_argument("--clients", type=int, nargs="+", default=[1, 8])
    parser.add_argument("--requests", type=int, default=500)
    parser.add_argument("--read-delay-ms", type=float, default=0)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if (
        min(args.sizes) < 0
        or min(args.files + args.clients + [args.requests]) < 1
        or args.read_delay_ms < 0
    ):
        parser.error(
            "Sizes must be nonnegative; files, clients and requests "
            "must be positive"
        )
    results = []
    with tempfile.TemporaryDirectory(prefix="doba-static-benchmark-") as folder:
        root = Path(folder)
        for size in args.sizes:
            for files in args.files:
                for i in range(files):
                    (root / f"{i}.bin").write_bytes(b"x" * size)
                for clients in args.clients:
                    for direct in (True, False):
                        result = measure(
                            args.binary.resolve(), root, size, files,
                            clients, args.requests, direct, args.read_delay_ms,
                        )
                        results.append(result)
                        print(json.dumps(result), flush=True)
    if args.output:
        args.output.write_text(json.dumps(results, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
