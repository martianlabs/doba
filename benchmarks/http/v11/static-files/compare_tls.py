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

"""Compare two builds on HttpArena's twenty real static TLS routes.

The supplied root must contain the official assets and their compressed siblings.
Both binaries use tls_server.cpp; the only variable is the library include tree.
"""
from __future__ import annotations

import argparse
import http.client
import json
from pathlib import Path
import re
import socket
import ssl
import subprocess
import sys
import tempfile
import time

sys.dont_write_bytecode = True

from benchmark import metrics

# Exact URI rotation from HttpArena requests/static-rotate.lua.
FILES = (
    "reset.css", "layout.css", "theme.css", "components.css", "utilities.css",
    "analytics.js", "helpers.js", "app.js", "vendor.js", "router.js",
    "header.html", "footer.html", "regular.woff2", "bold.woff2", "logo.svg",
    "icon-sprite.svg", "hero.webp", "thumb1.webp", "thumb2.webp", "manifest.json",
)


# +===========================================================================+
# | [>] measure                                                 ( function ) |
# +---------------------------------------------------------------------------+
def measure(binary, args, workers, script):
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
    command = [str(binary.resolve()), str(args.root.resolve()), str(workers),
               str(port), str(args.certificate.resolve()), str(args.key.resolve())]
    process = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    try:
        for _ in range(250):
            if process.poll() is not None:
                raise RuntimeError(process.stderr.read().decode())
            try:
                with socket.create_connection(("127.0.0.1", port), timeout=0.1):
                    break
            except OSError:
                time.sleep(0.02)
        else:
            raise RuntimeError("TLS server did not start")
        client = http.client.HTTPSConnection("127.0.0.1", port, timeout=10,
                                            context=ssl._create_unverified_context())
        try:
            for name in FILES:
                client.request("GET", "/static/" + name,
                               headers={"Accept-Encoding": "br;q=1, gzip;q=0.8"})
                response = client.getresponse()
                coding = response.getheader("Content-Encoding", "identity")
                suffix = {"br": ".br", "gzip": ".gz", "identity": ""}[coding]
                expected = (args.root / (name + suffix)).read_bytes()
                assert response.status == 200 and response.read() == expected, name
        finally:
            client.close()
        before = metrics(process)
        start = time.perf_counter()
        sample = subprocess.run(["wrk", "-t4", "-c1024", "-d" + args.duration,
                                 "-T5s", "--latency", "-s", str(script),
                                 f"https://127.0.0.1:{port}"],
                                text=True, capture_output=True, check=True)
        wall = time.perf_counter() - start
        after = metrics(process)
        requests = int(re.search(r"\n\s*(\d+) requests in", sample.stdout).group(1))
        return {"workers": workers, "requests": requests, "wall_seconds": wall,
                "rps_wall": requests / wall,
                "cpu_percent": 100 * (after["cpu_seconds"] - before["cpu_seconds"]) / wall,
                "rss_bytes": after["rss_bytes"], "peak_rss_bytes": after["peak_rss_bytes"],
                "payload_checks": len(FILES), "wrk": sample.stdout}
    finally:
        process.terminate()
        try:
            process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        process.stderr.close()


# +===========================================================================+
# | [>] main                                                ( entry-point ) |
# +---------------------------------------------------------------------------+
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("root", type=Path)
    parser.add_argument("certificate", type=Path)
    parser.add_argument("key", type=Path)
    parser.add_argument("--workers", type=int, nargs="+", default=[16, 64])
    parser.add_argument("--runs", type=int, default=3)
    parser.add_argument("--duration", default="5s")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.runs < 1 or min(args.workers) < 1:
        parser.error("Runs and worker counts must be positive")
    for name in FILES:
        if not (args.root / name).is_file():
            parser.error("Missing official asset: " + name)
    results = {"files": FILES, "connections": 1024, "runs": []}
    with tempfile.TemporaryDirectory(prefix="doba-static-routes-") as folder:
        script = Path(folder) / "rotate.lua"
        entries = ",\n".join(json.dumps("/static/" + name) for name in FILES)
        script.write_text("local paths = {" + entries + "}\nlocal counter = 0\n"
                          "request = function() counter = counter + 1; "
                          "return wrk.format('GET', paths[((counter-1)%20)+1], "
                          "{['Accept-Encoding']='br;q=1, gzip;q=0.8'}) end\n")
        for workers in args.workers:
            for run in range(args.runs):
                modes = [("before", args.baseline), ("after", args.candidate)]
                if run % 2:
                    modes.reverse()
                for mode, binary in modes:
                    row = measure(binary, args, workers, script)
                    row.update(mode=mode, run=run + 1)
                    results["runs"].append(row)
                    args.output.write_text(json.dumps(results, indent=2) + "\n")
                    print(json.dumps(row), flush=True)


if __name__ == "__main__":
    main()
