"""Generate binary HTTP seeds without relying on checkout newline conversion.

Grammar cases mirror decoder_tests.cpp, body/framer_chunked_tests.cpp,
body/reader_chunked_tests.cpp and integration/server_security_tests.cpp.
Only named seeds are overwritten; cached libFuzzer discoveries are retained.
"""

import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    successor = b"GET /next HTTP/1.1\r\nHost: example.com\r\n\r\n"
    chunks = {
        "empty": b"0\r\n\r\n",
        "payload": b"4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n",
        "extensions-trailers": b'3;name="a\\\"b"\r\nabc\r\n0\r\nX-Checksum: ok\r\n\r\n',
        "pipeline": b"1\r\na\r\n0\r\n\r\n" + successor,
        "incomplete": b"10\r\nabc",
        "bad-size": b"z\r\nx\r\n0\r\n\r\n",
        "overflow": b"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\r\n",
        "bad-data-crlf": b"1\r\naX\n0\r\n\r\n",
        "forbidden-trailer": b"0\r\nContent-Length: 4\r\n\r\n",
        "bad-trailer": b"0\r\nBad Name: value\r\n\r\n",
        "extension-limit": b"1;x=" + b"a" * 1100 + b"\r\na\r\n0\r\n\r\n",
        "trailer-limit": b"0\r\nX: " + b"a" * 4200 + b"\r\n\r\n",
        "payload-limit": b"1001\r\n" + b"a" * 4097 + b"\r\n0\r\n\r\n",
    }
    requests = {
        "get": successor,
        "pipeline": successor + successor,
        "query": b"GET /a?x=1&y=two%20words HTTP/1.1\r\nHost: example.com:80\r\n\r\n",
        "content-length": b"POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 4\r\n\r\nbody" + successor,
        "storage": b"POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 128\r\n\r\n" + b"a" * 128 + successor,
        "expect": b"POST / HTTP/1.1\r\nHost: x\r\nExpect: 100-continue\r\nContent-Length: 1\r\n\r\nx",
        "duplicate-length": b"POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 1\r\nContent-Length: 2\r\n\r\nx" + successor,
        "te-cl": b"POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 1\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n" + successor,
        "nul": b"GET / HTTP/1.1\r\nHost: x\r\nX: a\x00b\r\n\r\n",
        "missing-host": b"GET / HTTP/1.1\r\n\r\n",
        "bare-lf": b"GET / HTTP/1.1\nHost: x\n\n",
        "incomplete": b"POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 9\r\n\r\nabc",
        "head-limit": b"GET / HTTP/1.1\r\nHost: x\r\nX: " + b"a" * 4200 + b"\r\n\r\n",
        "query-limit": b"GET /?" + b"&".join([b"x=1"] * 33) + b" HTTP/1.1\r\nHost: x\r\n\r\n",
    }
    for name, chunk in chunks.items():
        requests["chunked-" + name] = (
            b"POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n" + chunk
        )
    for harness, corpus in (("decoder", requests), ("chunked", chunks)):
        directory = args.destination / harness
        directory.mkdir(parents=True, exist_ok=True)
        for name, wire in corpus.items():
            (directory / ("seed-" + name)).write_bytes(wire)
        print(f"{harness}: {len(corpus)} canonical seeds in {directory}")


if __name__ == "__main__":
    main()
