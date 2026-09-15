<a name="test-audit-and-implementation-results"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h1-test-audit-and-implementation-results-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h1-test-audit-and-implementation-results-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h1-test-audit-and-implementation-results-dark.svg">
    <img src="../resources/docs/test-audit/h1-test-audit-and-implementation-results.svg" alt="Test audit and implementation results">
  </picture>
</h1>

Date: 2026-09-07.
Baseline: `656171259afd2fdc5d9d469bb6ab1577f2539f2f`.
Implementation: local working-tree changes on that baseline.

The approved base scope is implemented and validated. The original N/R/O/H/Q
identifiers are retained below. This report replaces the pre-implementation
inventory with actual results; unresolved Q items remain explicit.
No production header, public API, dependency, root build file or CI workflow
was changed. No commit, remote CI execution or release is implied.

<a name="1-actual-counts-and-scope"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-1-actual-counts-and-scope-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-1-actual-counts-and-scope-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-1-actual-counts-and-scope-dark.svg">
    <img src="../resources/docs/test-audit/h2-1-actual-counts-and-scope.svg" alt="1. Actual counts and scope">
  </picture>
</h2>

| Suite | Baseline files | Baseline cases | New functional N | Reorganization O | Final files | Final cases |
| --- | --- | --- | --- | --- | --- | --- |
| Unit | 115 | 401 | 155 | 0 | 115 | 556 |
| Integration | 2 | 39 | 29 | 3 | 3 | 71 |
| Total | 117 | 440 | 184 | 3 | 118 | 627 |

The functional addition is **184 cases (41.8% of the baseline)**. The runner
registers 187 more cases (42.5%): three are the net effect of splitting one
existing HTTP test into four. That split contributes no new functional case.
HTTP integration now has 28 cases: four separated existing behaviors and
24 new scenarios. Transport retains 38 cases; the test client adds five.

There are **84 completed direct reinforcements** of existing cases. R-T04 is
the conditional 85th item and remains open with Q04. There are 21 named
auxiliary harness/lifetime checks, excluded from functional counts, plus an
isolated deferred-cleanup failure experiment. Data rows, split positions,
compiler runs, sanitizer runs and mutation experiments are also excluded.
The two TE dispatcher candidates under Q08 were not added or counted.

Counts were checked in source and in fresh runner `--list` output. Both
(file, test-name) inventories are unique: 556 unit and 71 integration entries.
The listing logs are `out/build/test-audit-gcc-debug/unit-list.log` and
`integration-list.log`. There is no measured line/branch coverage percentage
and no claim of exhaustive RFC or concurrency coverage.

<a name="2-findings-and-implemented-response"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-2-findings-and-implemented-response-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-2-findings-and-implemented-response-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-2-findings-and-implemented-response-dark.svg">
    <img src="../resources/docs/test-audit/h2-2-findings-and-implemented-response.svg" alt="2. Findings and implemented response">
  </picture>
</h2>

| Finding | Confirmed cause or gap | Response / status |
| --- | --- | --- |
| 1. Fixture lifetime | Captured state could be destroyed before the server on assertion return. A suspended coroutine could also outlive cleanup. | Reviewed all 38 TCP fixtures; state precedes server. Ten deferred fixtures have a private cleanup guard. Active-transport and suspended-failure probes validated teardown. |
| 2. Executor concurrency | Producer and consumer phases did not prove overlapping work. | R-E02 uses four producers and four consumers with a barrier and overlap handshake; N-E01/N-E02 exercise stop and nested scheduling. |
| 3. HTTP integration volume | One aggregated HTTP integration case left ordering, body and lifetime paths unprotected. | N-I01..24, R-I01/O01 and R-D11. |
| 4. Header dispatch | Direct field checks did not prove decoder dispatch reached each validator. | 100 common-field and 16 modeled-field positive/negative cases; six literal variants each. Q08 remains open. |
| 5. Memory-to-file transition | Large writes alone did not establish preservation of a pre-existing memory prefix. | N-S01..06, N-D08/N-D09 and N-I10/N-I11 cross thresholds incrementally and compare exact bytes. |
| 6. Exactly once | Aggregate counters could hide an omission plus a duplicate. | Per-handle executor counts; TCP totals after stop/join and individual client categories in R-T06. |
| 7. Output and backpressure | Uniform payloads hid offset errors; serialization was not evidence of a blocked send. | Position-dependent binary patterns and exact sequences. Q04/R-T04 remain open. |
| 8. Weak byte oracles | Substring/EOF checks could miss over-consumption or extra output. | Exact NEXT leftovers, complete bodies, no-body framing and independent socket response delimitation. |
| 9. Limits | Some tests stopped at incomplete input instead of exercising a complete boundary. | Complete head, query, extension, trailer, inline-body and spill boundaries. Q02/Q07 remain open. |
| 10. Negative fragmentation | Malformed requests were mainly checked as complete buffers. | 43 malformed inputs across 3991 scheduled delivery layouts; 23 prefix-sensitive request lines. |
| 11. Harness diagnostics | Assertions discarded location/expression; exceptions could terminate the runner. | Per-case exception reporting and continuation, row context, active-case flush, filters and isolated probes. |
| 12. Bounds and progress | Read loops and test-client network operations could wait without an overall bound. | Bounded reader loops, absolute client deadlines and a 120-second CTest process timeout. Q04/Q06 remain open. |
| 13. Vacuous view tests | 85 assertions in 17 files used double-escaped text instead of the intended control bytes. | Corrected literals; 52 boundary tests now mutate real bytes and validate bounded views. |
| 14. Storage ownership | Fixture removal could hide missing library cleanup; moves/cursors lacked coverage. | R-S01/R-S02 and N-S07..17 verify files before fixture cleanup, ownership and cursor state. Q03 remains open. |
| 15. Route conversions | Important accepted and rejected conversions lacked checks across all entry points. | N-P01..08: 28 inputs across matches, invoke and invoke_async, including exact values/callback counts/errors. |
| 16. Dates and lifecycle | Thread startup could consume the observation window; shape-only dates and anchor ownership weakened coverage. | R-F01..03 and N-F01/N-F02: nonzero per-thread work, calendar/weekday checks, restart and transitions without an anchor. |
| 17. Host and absolute-form | Routing rejects a target authority that differs from Host. | Reproduced; existing expectation retained pending the localized Q01 correction. |
| 18. Conditional dates | A purported invalid date was actually a valid obsolete format. | R-D01 uses not-a-date for both fields; N-D02 separately accepts all three valid formats. |
| A. Port reservation | find_available_port closes its reservation before server bind. | Q05 remains open; no claim that loopback or successful runs remove the race. |
| B. Client I/O | Blocking calls did not enforce a whole-operation deadline or distinguish outcomes. | Nonblocking connect/send/receive with select and absolute deadlines; N-C01..05. Pending-connect and proven blocked-send checks remain Q06/Q04. |

The byte-view fixes use actual NUL/CR/LF/DEL and high-byte values. For example,
the old source literal `"\\0"` exposed a backslash; the corrected `"\0"`
exposes a NUL. The malformed-input expectations were strengthened, not
weakened. Existing Host/TE contract conflicts were not silently reclassified.

<a name="3-new-case-traceability"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-3-new-case-traceability-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-3-new-case-traceability-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-3-new-case-traceability-dark.svg">
    <img src="../resources/docs/test-audit/h2-3-new-case-traceability.svg" alt="3. New-case traceability">
  </picture>
</h2>

Every N item below is **implemented and passed in all ten configurations**
in section 7. File/name selects the exact case with `--file` and `--name`.
Rows inside a case are input evaluations, not additional registrations.

<a name="n-h0150-pn-common-header-dispatch"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-n-h0150-pn-common-header-dispatch-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-n-h0150-pn-common-header-dispatch-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-n-h0150-pn-common-header-dispatch-dark.svg">
    <img src="../resources/docs/test-audit/h3-n-h0150-pn-common-header-dispatch.svg" alt="N-H01..50-P/N: common header dispatch">
  </picture>
</h3>

File: [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp).
For each field F, `N-Hxx-P` is named `decoder accepts valid F`, and
`N-Hxx-N` is named `decoder rejects invalid F`. These names use the field
spelling in the table. Each case executes canonical/lower/upper field names,
each without OWS and with HTAB/SP: six requests. The positive oracle is
success, a present request and the exact normalized value; the negative is
invalid source with no request. Expectations are literal, not computed by
calling the production field checker. Total: 100 registrations, 600 requests.

| ID pair | Field F | Accepted value | Rejected value |
| --- | --- | --- | --- |
| N-H01-P/N | `Accept` | `text/plain` | `text/` |
| N-H02-P/N | `Accept-Charset` | `utf-8` | `utf-8;q=1.001` |
| N-H03-P/N | `Accept-Encoding` | `gzip` | `gzip;q=1.1` |
| N-H04-P/N | `Accept-Language` | `en-US` | `en--US` |
| N-H05-P/N | `Accept-Ranges` | `bytes` | `byte range` |
| N-H06-P/N | `Access-Control-Allow-Credentials` | `true` | `false` |
| N-H07-P/N | `Access-Control-Allow-Headers` | `Content-Type` | `Content Type` |
| N-H08-P/N | `Access-Control-Allow-Methods` | `GET` | `GET POST` |
| N-H09-P/N | `Access-Control-Allow-Origin` | `https://example.com` | `https://example.com/path` |
| N-H10-P/N | `Access-Control-Expose-Headers` | `ETag` | `Content Length` |
| N-H11-P/N | `Access-Control-Max-Age` | `600` | `600s` |
| N-H12-P/N | `Access-Control-Request-Headers` | `Content-Type` | `Content Type` |
| N-H13-P/N | `Access-Control-Request-Method` | `GET` | `GET POST` |
| N-H14-P/N | `Age` | `000` | `1s` |
| N-H15-P/N | `Allow` | `GET` | `GET POST` |
| N-H16-P/N | `Authentication-Info` | `nextnonce="abc"` | `nextnonce=` |
| N-H17-P/N | `Authorization` | `Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==` | `Basic "abc"` |
| N-H18-P/N | `Cache-Control` | `max-age=60` | `max-age=` |
| N-H19-P/N | `Content-Encoding` | `gzip` | `g zip` |
| N-H20-P/N | `Content-Language` | `en-US` | `en--US` |
| N-H21-P/N | `Content-Location` | `/index.html` | `%GG` |
| N-H22-P/N | `Content-Range` | `bytes 0-0/1` | `bytes 0-1/` |
| N-H23-P/N | `Content-Type` | `text/plain` | `text` |
| N-H24-P/N | `Cookie` | `a=b` | `=b` |
| N-H25-P/N | `Date` | `Sun, 06 Nov 1994 08:49:37 GMT` | `Sun, 06 Nov 1994 08:49:37 UTC` |
| N-H26-P/N | `ETag` | `"abc"` | `abc` |
| N-H27-P/N | `Expires` | `Sun, 06 Nov 1994 08:49:37 GMT` | `Sun, 06 Nov 1994 08:49:37 UTC` |
| N-H28-P/N | `From` | `user@example.com` | `user@` |
| N-H29-P/N | `If-Match` | `"abc"` | `abc` |
| N-H30-P/N | `If-None-Match` | `"abc"` | `abc` |
| N-H31-P/N | `If-Range` | `"abc"` | `abc` |
| N-H32-P/N | `Keep-Alive` | `timeout=5` | `timeout=` |
| N-H33-P/N | `Last-Modified` | `Sun, 06 Nov 1994 08:49:37 GMT` | `Sun, 06 Nov 1994 08:49:37 UTC` |
| N-H34-P/N | `Location` | `/index.html` | `%GG` |
| N-H35-P/N | `Origin` | `https://example.com` | `https://example.com/path` |
| N-H36-P/N | `Pragma` | `no-cache` | `foo=` |
| N-H37-P/N | `Proxy-Connection` | `close` | `keep alive` |
| N-H38-P/N | `Range` | `bytes=0-499` | `bytes=0--1` |
| N-H39-P/N | `Referer` | `/index.html` | `%GG` |
| N-H40-P/N | `Retry-After` | `120` | `120s` |
| N-H41-P/N | `Sec-WebSocket-Accept` | `s3pPLMBiTxaQ9kYGzzhZRbK+xOo=` | `A===` |
| N-H42-P/N | `Sec-WebSocket-Extensions` | `permessage-deflate` | `extension;name=` |
| N-H43-P/N | `Sec-WebSocket-Key` | `dGhlIHNhbXBsZSBub25jZQ==` | `A===` |
| N-H44-P/N | `Sec-WebSocket-Protocol` | `chat` | `chat protocol` |
| N-H45-P/N | `Sec-WebSocket-Version` | `255` | `256` |
| N-H46-P/N | `Server` | `doba/1.0` | `doba/` |
| N-H47-P/N | `Set-Cookie` | `a=b` | `=b` |
| N-H48-P/N | `User-Agent` | `doba/1.0` | `doba/` |
| N-H49-P/N | `Vary` | `Accept-Encoding` | `Accept Encoding` |
| N-H50-P/N | `WWW-Authenticate` | `Basic realm = "x"` | `Basic =value` |

Response-associated fields already appear in the decoder map. These cases
protect that validation contract; they do not imply application support for
CORS, authentication, caching, WebSocket or protocol switching. Invalid
conditional dates follow their existing ignore contract in R-D01, separately.

<a name="n-m0209-pn-modeled-header-dispatch"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-n-m0209-pn-modeled-header-dispatch-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-n-m0209-pn-modeled-header-dispatch-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-n-m0209-pn-modeled-header-dispatch-dark.svg">
    <img src="../resources/docs/test-audit/h3-n-m0209-pn-modeled-header-dispatch.svg" alt="N-M02..09-P/N: modeled header dispatch">
  </picture>
</h3>

Same file, six name/OWS variants and naming rule as N-H. All 16 cases passed
(96 requests). The context shown is used for both members of each pair.

| ID pair | Field | Accepted value | Rejected value | Request context |
| --- | --- | --- | --- | --- |
| N-M02-P/N | Trailer | X-Checksum | Content Length | POST chunked, complete zero chunk and X-Checksum: abc trailer. |
| N-M03-P/N | Upgrade | websocket | HTTP/ | GET, Connection: upgrade; existing allow_upgrade default. |
| N-M04-P/N | Max-Forwards | 10 | +1 | OPTIONS *. |
| N-M05-P/N | Via | 1.1 proxy | 1.1 proxy (unterminated | GET. |
| N-M06-P/N | Forwarded | for=192.0.2.43;proto=http | for= | GET. |
| N-M07-P/N | X-Forwarded-For | 192.0.2.1 | 999.0.0.1 | GET. |
| N-M08-P/N | X-Forwarded-Host | example.com:80 | example.com:http | GET. |
| N-M09-P/N | X-Forwarded-Proto | https | 1http | GET. |

N-M01-P/N remains reserved for Q08 and is absent from these totals.

<a name="remaining-68-new-cases"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-remaining-68-new-cases-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-remaining-68-new-cases-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-remaining-68-new-cases-dark.svg">
    <img src="../resources/docs/test-audit/h3-remaining-68-new-cases.svg" alt="Remaining 68 new cases">
  </picture>
</h3>

The following tables give exact registered names and executed inputs. All
route rows run through matches, invoke and invoke_async. Negative route rows
require no callback and the existing async conversion error; positive rows
require exact values and one callback per invocation.

<a name="decoder"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-decoder-dark.svg">
    <img src="../resources/docs/test-audit/h4-decoder.svg" alt="Decoder">
  </picture>
</h4>

[tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-D01 | `decoder reports unsupported expectations before body arrival` | Unsupported Expect feature=on before body arrival; exact expectation-failed reason and no interim. |
| N-D02 | `decoder accepts all conditional date formats` | Both conditional fields with IMF-fixdate, RFC850 and asctime: six valid values. |
| N-D03 | `decoder accepts complete heads at the buffer boundary` | Complete heads of 5119 and 5120 bytes; exact consumption and untruncated value. |
| N-D04 | `decoder preserves every query pair at the supported limit` | 127 and 128 unique query pairs; every key/value, including the final pair. |
| N-D05 | `decoder preserves a pipelined successor after a raw body` | Seven-byte raw body followed by GET /next; all 112 two-fragment cuts. |
| N-D06 | `decoder preserves a pipelined successor after a chunked body` | Chunked abc followed by GET /next; all 127 two-fragment cuts. |
| N-D07 | `decoder accepts every target form byte by byte` | Four accepted request-target forms, bytewise; no premature dispatch. |
| N-D08 | `decoder preserves raw bodies across the spill threshold` | Raw body sizes 65534/65535/65536, position-dependent binary data, input blocks <=4096. |
| N-D09 | `decoder preserves encoded chunked bodies across the spill threshold` | Encoded chunked body sizes 65534/65535/65536; exact decoded payload after incremental accumulation. |

<a name="storage-and-reader-ownership"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h4-storage-and-reader-ownership-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h4-storage-and-reader-ownership-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-storage-and-reader-ownership-dark.svg">
    <img src="../resources/docs/test-audit/h4-storage-and-reader-ownership.svg" alt="Storage and reader ownership">
  </picture>
</h4>

[tests/unit/common/byte_storage_tests.cpp](../tests/unit/common/byte_storage_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-S01 | `storage below the threshold remains in memory` | Threshold 8, six bytes: memory only, exact data and EOF. |
| N-S02 | `storage at the threshold remains in memory` | Threshold 8, eight bytes: no spill at equality. |
| N-S03 | `crossing the threshold preserves the memory prefix` | Threshold 8, writes abcdef and GHIJ: one spill, exact ten bytes. |
| N-S04 | `zero threshold disables spilling` | Threshold zero, repeated writes totaling 32 bytes: spilling disabled. |
| N-S05 | `successive spill writes append without duplication` | Writes abcdef, GHIJ, klmno: one file and ordered append. |
| N-S06 | `storage preserves every byte value` | All 256 byte values through memory and incremental spill. |
| N-S07 | `memory move construction preserves the cursor` | Memory move construction after reading two bytes: cursor preserved. |
| N-S08 | `spill move construction transfers cursor and ownership` | Spill move construction after reading two bytes: cursor and file ownership preserved. |
| N-S09 | `spill move assignment releases the previous file` | Spill-to-spill move assignment removes the old destination file before fixture cleanup. |
| N-S10 | `memory move assignment releases a spilled destination` | Memory-to-spill destination move assignment removes the previous file. |
| N-S11 | `spill move assignment replaces memory and preserves cursor` | Spill-to-memory destination move assignment transfers file, cursor and bytes. |
| N-S12 | `spill creation failure remains observable` | Spill directory names a regular file: failure remains visible on later operations. |
| N-S13 | `read and fetch share one storage cursor` | read(2), fetch(1), read(rest) on abcdef: ab/c/def and one shared cursor in both backends. |
| N-S14 | `reader owns storage after the moved source is destroyed` | Reader owns moved storage after source destruction; memory and spill cleanup. |
| N-S15 | `concurrent spill owners keep distinct files and contents` | Eight simultaneous spill owners: distinct paths, independent content, cleanup before fixture destruction. |
| N-S16 | `empty storage finishes without an error` | Empty write and finish: size zero, completed reader, no error. |
| N-S17 | `moving a borrowed reader preserves the view and cursor` | Borrowed reader moves after reading ab; live backing c changes to C and remaining bytes are Cdef. |

<a name="chunked-reader"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-chunked-reader-dark.svg">
    <img src="../resources/docs/test-audit/h4-chunked-reader.svg" alt="Chunked reader">
  </picture>
</h4>

[tests/unit/protocol/http/v11/body/reader_chunked_tests.cpp](../tests/unit/protocol/http/v11/body/reader_chunked_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-B01 | `chunked reader preserves the following request bytes` | Chunked a with a trailer followed by NEXT; output capacities 1/8, exact successor and EOF. |

<a name="executor"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-executor-dark.svg">
    <img src="../resources/docs/test-audit/h4-executor.svg" alt="Executor">
  </picture>
</h4>

[tests/unit/transport/server/executor_tests.cpp](../tests/unit/transport/server/executor_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-E01 | `executor stop separates accepted and rejected concurrent work` | Concurrent producers/consumers overlap stop; accepted handles run once, rejected handles never run; drain/restart. |
| N-E02 | `executor continuations can schedule a successor` | A running continuation schedules a successor; both run once and queue drains without deadlock. |

<a name="route-conversion"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-route-conversion-dark.svg">
    <img src="../resources/docs/test-audit/h4-route-conversion.svg" alt="Route conversion">
  </picture>
</h4>

[tests/unit/protocol/http/common/router_handler_parametrized_tests.cpp](../tests/unit/protocol/http/common/router_handler_parametrized_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-P01 | `route conversion accepts every boolean spelling` | false/FALSE/FaLsE/0/1; exact booleans. |
| N-P02 | `route conversion accepts signed integer boundaries` | Signed 64-bit min/max, -1 and zero. |
| N-P03 | `route conversion accepts unsigned integer boundaries` | Unsigned 64-bit zero and maximum. |
| N-P04 | `route conversion rejects integer overflow` | Signed below-min/above-max and unsigned above-max. |
| N-P05 | `route conversion rejects partial numbers spaces and plus signs` | 42x, 1.5x, leading/trailing spaces, +42 and +1.5: reject partial or disallowed spellings. |
| N-P06 | `route conversion rejects negative unsigned values` | Unsigned -1 and -0 rejected. |
| N-P07 | `route conversion accepts finite floating point values` | -1.5, 1e2, 0 and 1.5e-2: exact finite values. |
| N-P08 | `route conversion rejects floating point range errors` | 1e9999 and 1e-9999 rejected for range. |

<a name="date-lifecycle"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-date-lifecycle-dark.svg">
    <img src="../resources/docs/test-audit/h4-date-lifecycle.svg" alt="Date lifecycle">
  </picture>
</h4>

[tests/unit/common/date_server_tests.cpp](../tests/unit/common/date_server_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-F01 | `date server restarts after the last owner stops` | Three start/update/last-stop cycles without an outside owner; valid advancing dates. |
| N-F02 | `date server handles concurrent last stop and first start` | Three simultaneous last-stop/first-start transitions without an anchor; surviving owner updates. |

<a name="http-over-real-sockets"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-http-over-real-sockets-dark.svg">
    <img src="../resources/docs/test-audit/h4-http-over-real-sockets.svg" alt="HTTP over real sockets">
  </picture>
</h4>

[tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-I01 | `HTTP/1.1 reuses a connection for sequential requests` | Sequential requests reuse one connection; exact responses. |
| N-I02 | `HTTP/1.1 orders three synchronous pipelined responses` | Three synchronous pipelined requests retain response order. |
| N-I03 | `HTTP/1.1 keeps synchronous responses behind a suspended handler` | First handler suspended while the second runs; responses still follow request order. |
| N-I04 | `HTTP/1.1 orders asynchronous responses by request order` | Asynchronous handlers complete in reverse order; wire responses remain in request order. |
| N-I05 | `HTTP/1.1 preserves pipeline framing after a raw body` | Raw request body followed by another request; exact bodies and framing. |
| N-I06 | `HTTP/1.1 preserves pipeline framing after a chunked body` | Chunked request body followed by another request; exact bodies and framing. |
| N-I07 | `HTTP/1.1 waits for the final head delimiter` | Head missing its final delimiter is not dispatched; final fragment completes it. |
| N-I08 | `HTTP/1.1 waits for a complete fragmented raw body` | Fragmented raw body is dispatched only when complete. |
| N-I09 | `HTTP/1.1 waits for a complete fragmented chunked body` | Fragmented chunked body is dispatched only when complete. |
| N-I10 | `HTTP/1.1 echoes raw bodies across the spill threshold` | Raw bodies across 65535-byte spill threshold; complete echoed bytes. |
| N-I11 | `HTTP/1.1 echoes chunked bodies across the spill threshold` | Chunked encoded bodies across 65535-byte spill threshold; complete decoded echo. |
| N-I12 | `HTTP/1.1 emits no HEAD body before the following GET` | HEAD emits no payload before the next GET response. |
| N-I13 | `HTTP/1.1 delimits a 204 before a following response` | 204 emits no payload before the following response. |
| N-I14 | `HTTP/1.1 rejects invalid header syntax and its successor` | Invalid header syntax plus successor: 400, exact error body, EOF and zero handler calls. |
| N-I15 | `HTTP/1.1 rejects invalid dispatched header values and its successor` | Invalid dispatched field value plus successor: 400, exact error body, EOF and zero handler calls. |
| N-I16 | `HTTP/1.1 retains suspended request views across later heads` | Suspended first request retains views while two later heads carry 4000-byte field values. |
| N-I17 | `HTTP/1.1 drains a complete request after a half close` | Complete request followed by a write half-close; response drains. |
| N-I18 | `HTTP/1.1 closes incomplete raw input without dispatch` | Incomplete raw input followed by EOF; no handler dispatch. |
| N-I19 | `HTTP/1.1 closes incomplete chunked input without dispatch` | Incomplete chunked input followed by EOF; no handler dispatch. |
| N-I20 | `HTTP/1.1 preserves binary raw payload bytes` | Binary raw body including NUL and high bytes is echoed exactly. |
| N-I21 | `HTTP/1.1 omits interim responses when the body is already complete` | Expect body already supplied: no interim response. |
| N-I22 | `HTTP/1.1 sends one interim while a fragmented body is pending` | Pending fragmented Expect body: exactly one interim, then final response. |
| N-I23 | `HTTP/1.1 completes a large response before a short successor` | 131089-byte position-dependent response followed by a short response; exact sequence. |
| N-I24 | `HTTP/1.1 cancels a suspended request after client reset` | Reset cancels suspended work, releases the retained request, and leaves the server usable. |

<a name="integration-client"></a>
<h4>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h4-integration-client-dark.svg">
    <img src="../resources/docs/test-audit/h4-integration-client.svg" alt="Integration client">
  </picture>
</h4>

[tests/integration/tcpip_client_tests.cpp](../tests/integration/tcpip_client_tests.cpp)

| ID | Registered name | Executed input / assertion |
| --- | --- | --- |
| N-C01 | `client reports refused connections` | Local bound but non-listening port; socket refusal distinct from timeout, bounded by the three-second default. |
| N-C02 | `client bounds silent receive operations` | Connected silent peer; 200 ms receive timeout and timeout diagnostic. |
| N-C03 | `client preserves the deadline after a partial receive` | Partial input then no completion; one 600 ms deadline, not a new budget after progress. |
| N-C04 | `client bounds receive until close despite progress` | Peer drips bytes without closing; receive_until_close expires at one 250 ms deadline. |
| N-C05 | `client distinguishes eof from reset` | Orderly EOF and reset produce distinct EOF/socket diagnostics. |

<a name="4-existing-case-reinforcements"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-4-existing-case-reinforcements-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-4-existing-case-reinforcements-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-4-existing-case-reinforcements-dark.svg">
    <img src="../resources/docs/test-audit/h2-4-existing-case-reinforcements.svg" alt="4. Existing-case reinforcements">
  </picture>
</h2>

<a name="r-h0152-byte-views"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-r-h0152-byte-views-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-r-h0152-byte-views-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-r-h0152-byte-views-dark.svg">
    <img src="../resources/docs/test-audit/h3-r-h0152-byte-views.svg" alt="R-H01..52: byte views">
  </picture>
</h3>

Each row names an existing `check handles string view boundaries` case in
`tests/unit/protocol/http/common/headers/`. All 52 are reinforced and passed
in every configuration in section 7. Beginning, middle and last byte
positions are distinct for each listed seed. Each position is replaced with
NUL, CR, LF and DEL: 624 mutation evaluations. Another 52 evaluations expose
only the valid seed through a sized view over backing storage followed by
an actual NUL and suffix.

Accept, Content-Type, Cache-Control, ETag and User-Agent each compare allowed
0x80 in quoted/opaque/comment content with a disallowed token position.
Cookie compares a=b with a high-byte cookie value. Those twelve evaluations
bring this matrix to 688. Some positive seeds/obs-text spellings had prior
coverage; 688 is an evaluation count, not 688 newly protected inputs.

| ID | Existing file | Valid seed |
| --- | --- | --- |
| R-H01 | [accept_tests.cpp](../tests/unit/protocol/http/common/headers/accept_tests.cpp) | `text/plain` |
| R-H02 | [accept_charset_tests.cpp](../tests/unit/protocol/http/common/headers/accept_charset_tests.cpp) | `utf-8` |
| R-H03 | [accept_encoding_tests.cpp](../tests/unit/protocol/http/common/headers/accept_encoding_tests.cpp) | `gzip` |
| R-H04 | [accept_language_tests.cpp](../tests/unit/protocol/http/common/headers/accept_language_tests.cpp) | `en-US` |
| R-H05 | [accept_ranges_tests.cpp](../tests/unit/protocol/http/common/headers/accept_ranges_tests.cpp) | `bytes` |
| R-H06 | [access_control_allow_credentials_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_allow_credentials_tests.cpp) | `true` |
| R-H07 | [access_control_allow_headers_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_allow_headers_tests.cpp) | `Content-Type` |
| R-H08 | [access_control_allow_methods_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_allow_methods_tests.cpp) | `GET` |
| R-H09 | [access_control_allow_origin_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_allow_origin_tests.cpp) | `https://example.com` |
| R-H10 | [access_control_expose_headers_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_expose_headers_tests.cpp) | `ETag` |
| R-H11 | [access_control_max_age_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_max_age_tests.cpp) | `600` |
| R-H12 | [access_control_request_headers_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_request_headers_tests.cpp) | `Content-Type` |
| R-H13 | [access_control_request_method_tests.cpp](../tests/unit/protocol/http/common/headers/access_control_request_method_tests.cpp) | `GET` |
| R-H14 | [age_tests.cpp](../tests/unit/protocol/http/common/headers/age_tests.cpp) | `000` |
| R-H15 | [allow_tests.cpp](../tests/unit/protocol/http/common/headers/allow_tests.cpp) | `GET` |
| R-H16 | [authentication_info_tests.cpp](../tests/unit/protocol/http/common/headers/authentication_info_tests.cpp) | `nextnonce="abc"` |
| R-H17 | [authorization_tests.cpp](../tests/unit/protocol/http/common/headers/authorization_tests.cpp) | `Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==` |
| R-H18 | [cache_control_tests.cpp](../tests/unit/protocol/http/common/headers/cache_control_tests.cpp) | `max-age=60` |
| R-H19 | [content_encoding_tests.cpp](../tests/unit/protocol/http/common/headers/content_encoding_tests.cpp) | `gzip` |
| R-H20 | [content_language_tests.cpp](../tests/unit/protocol/http/common/headers/content_language_tests.cpp) | `en-US` |
| R-H21 | [content_location_tests.cpp](../tests/unit/protocol/http/common/headers/content_location_tests.cpp) | `/index.html` |
| R-H22 | [content_range_tests.cpp](../tests/unit/protocol/http/common/headers/content_range_tests.cpp) | `bytes 0-0/1` |
| R-H23 | [content_type_tests.cpp](../tests/unit/protocol/http/common/headers/content_type_tests.cpp) | `text/plain` |
| R-H24 | [cookie_tests.cpp](../tests/unit/protocol/http/common/headers/cookie_tests.cpp) | `a=b` |
| R-H25 | [date_tests.cpp](../tests/unit/protocol/http/common/headers/date_tests.cpp) | `Sun, 06 Nov 1994 08:49:37 GMT` |
| R-H26 | [etag_tests.cpp](../tests/unit/protocol/http/common/headers/etag_tests.cpp) | `"abc"` |
| R-H27 | [expires_tests.cpp](../tests/unit/protocol/http/common/headers/expires_tests.cpp) | `Sun, 06 Nov 1994 08:49:37 GMT` |
| R-H28 | [from_tests.cpp](../tests/unit/protocol/http/common/headers/from_tests.cpp) | `user@example.com` |
| R-H29 | [if_match_tests.cpp](../tests/unit/protocol/http/common/headers/if_match_tests.cpp) | `"abc"` |
| R-H30 | [if_none_match_tests.cpp](../tests/unit/protocol/http/common/headers/if_none_match_tests.cpp) | `"abc"` |
| R-H31 | [if_range_tests.cpp](../tests/unit/protocol/http/common/headers/if_range_tests.cpp) | `"abc"` |
| R-H32 | [keep_alive_tests.cpp](../tests/unit/protocol/http/common/headers/keep_alive_tests.cpp) | `timeout=5` |
| R-H33 | [last_modified_tests.cpp](../tests/unit/protocol/http/common/headers/last_modified_tests.cpp) | `Sun, 06 Nov 1994 08:49:37 GMT` |
| R-H34 | [location_tests.cpp](../tests/unit/protocol/http/common/headers/location_tests.cpp) | `/index.html` |
| R-H35 | [origin_tests.cpp](../tests/unit/protocol/http/common/headers/origin_tests.cpp) | `https://example.com` |
| R-H36 | [pragma_tests.cpp](../tests/unit/protocol/http/common/headers/pragma_tests.cpp) | `no-cache` |
| R-H37 | [proxy_connection_tests.cpp](../tests/unit/protocol/http/common/headers/proxy_connection_tests.cpp) | `close` |
| R-H38 | [range_tests.cpp](../tests/unit/protocol/http/common/headers/range_tests.cpp) | `bytes=0-499` |
| R-H39 | [referer_tests.cpp](../tests/unit/protocol/http/common/headers/referer_tests.cpp) | `/index.html` |
| R-H40 | [retry_after_tests.cpp](../tests/unit/protocol/http/common/headers/retry_after_tests.cpp) | `120` |
| R-H41 | [sec_websocket_accept_tests.cpp](../tests/unit/protocol/http/common/headers/sec_websocket_accept_tests.cpp) | `s3pPLMBiTxaQ9kYGzzhZRbK+xOo=` |
| R-H42 | [sec_websocket_extensions_tests.cpp](../tests/unit/protocol/http/common/headers/sec_websocket_extensions_tests.cpp) | `permessage-deflate` |
| R-H43 | [sec_websocket_key_tests.cpp](../tests/unit/protocol/http/common/headers/sec_websocket_key_tests.cpp) | `dGhlIHNhbXBsZSBub25jZQ==` |
| R-H44 | [sec_websocket_protocol_tests.cpp](../tests/unit/protocol/http/common/headers/sec_websocket_protocol_tests.cpp) | `chat` |
| R-H45 | [sec_websocket_version_tests.cpp](../tests/unit/protocol/http/common/headers/sec_websocket_version_tests.cpp) | `255` |
| R-H46 | [server_tests.cpp](../tests/unit/protocol/http/common/headers/server_tests.cpp) | `doba/1.0` |
| R-H47 | [set_cookie_tests.cpp](../tests/unit/protocol/http/common/headers/set_cookie_tests.cpp) | `a=b` |
| R-H48 | [user_agent_tests.cpp](../tests/unit/protocol/http/common/headers/user_agent_tests.cpp) | `doba/1.0` |
| R-H49 | [vary_tests.cpp](../tests/unit/protocol/http/common/headers/vary_tests.cpp) | `Accept-Encoding` |
| R-H50 | [www_authenticate_tests.cpp](../tests/unit/protocol/http/common/headers/www_authenticate_tests.cpp) | `Basic realm = "x"` |
| R-H51 | [if_modified_since_tests.cpp](../tests/unit/protocol/http/common/headers/if_modified_since_tests.cpp) | `Sun, 06 Nov 1994 08:49:37 GMT` |
| R-H52 | [if_unmodified_since_tests.cpp](../tests/unit/protocol/http/common/headers/if_unmodified_since_tests.cpp) | `Sun, 06 Nov 1994 08:49:37 GMT` |

<a name="remaining-33-r-items"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-remaining-33-r-items-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-remaining-33-r-items-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-remaining-33-r-items-dark.svg">
    <img src="../resources/docs/test-audit/h3-remaining-33-r-items.svg" alt="Remaining 33 R items">
  </picture>
</h3>

All rows except R-T04 are completed and passed under the full matrix. R-T04
still passes its pre-existing assertion but does not satisfy its stronger
proposed backpressure criterion. It is excluded from the 84 completed total.

| ID | File and exact existing name | Evidence / remaining limit |
| --- | --- | --- |
| R-D01 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `invalid conditional dates do not reject requests` | Both invalid inputs are not-a-date; valid obsolete date spellings moved to explicit positive coverage. |
| R-D02 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `rejects malformed request line and header syntax` | 15 malformed request/head inputs; two-fragment, selected three-fragment and bytewise delivery. |
| R-D03 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `distinguishes incomplete and terminal invalid request lines` | 23 request-line inputs with literal earliest-invalid-byte thresholds; every prefix plus bytewise delivery. |
| R-D04 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `rejects invalid cross header combinations` | Nine cross-field failures across delivery layouts; the existing absolute-form/Host mismatch remains a separate unchanged expectation under Q01. |
| R-D05 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `rejects transfer coding without final chunked` | Transfer-Encoding: gzip without final chunked across delivery layouts. |
| R-D06 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `rejects embedded null and control bytes` | Four real embedded-NUL positions across delivery layouts. |
| R-D07 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `rejects malformed chunked framing` | Six malformed chunk bodies across delivery layouts. |
| R-D08 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `rejects malformed chunk extensions` | Malformed extension across delivery layouts. |
| R-D09 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `rejects malformed chunk trailers` | Two malformed trailer bodies across delivery layouts. |
| R-D10 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `reports version specific rejection reasons` | Five protocol versions across delivery layouts; exact rejection reason retained. |
| R-D11 | [tests/unit/protocol/http/v11/decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp) - `pipelined requests remain available after first dispatch` | First request views remain exact after the second dispatch and eight later decoder reloads. |
| R-B01 | [tests/unit/protocol/http/v11/body/reader_raw_tests.cpp](../tests/unit/protocol/http/v11/body/reader_raw_tests.cpp) - `reads exactly content length for every output size` | Bounded progress, complete payload, exact NEXT bytes and final EOF for every output size. |
| R-B02 | [tests/unit/protocol/http/v11/body/reader_tests.cpp](../tests/unit/protocol/http/v11/body/reader_tests.cpp) - `raw factory decodes only the declared body` | Bounded progress and explicit completion for raw factory output. |
| R-B03 | [tests/unit/protocol/http/v11/body/reader_tests.cpp](../tests/unit/protocol/http/v11/body/reader_tests.cpp) - `chunked factory removes wire framing and trailers` | Bounded progress and explicit completion for chunked factory output. |
| R-B04 | [tests/unit/protocol/http/v11/body/reader_chunked_tests.cpp](../tests/unit/protocol/http/v11/body/reader_chunked_tests.cpp) - `decodes chunks extensions and trailers` | Bounded iterations plus explicit completion and exact decoded bytes. |
| R-B05 | [tests/unit/protocol/http/v11/body/framer_chunked_tests.cpp](../tests/unit/protocol/http/v11/body/framer_chunked_tests.cpp) - `enforces extension and trailer size limits` | Six complete extension/trailer boundary inputs, every two-fragment cut, exact consumption/output or persistent specific error. |
| R-B06 | [tests/unit/protocol/http/v11/body/reader_chunked_tests.cpp](../tests/unit/protocol/http/v11/body/reader_chunked_tests.cpp) - `enforces extension and trailer size limits` | The same six complete boundary inputs at output capacities 1 and 8; exact body/EOF or persistent specific error. |
| R-O01 | [tests/unit/protocol/http/v11/response_tests.cpp](../tests/unit/protocol/http/v11/response_tests.cpp) - `informational 204 205 and 304 responses never serialize bodies` | No bytes after the head and no source for informational/204/205/304; exact Content-Length presence/value by existing contract. |
| R-O02 | [tests/unit/protocol/http/v11/response_tests.cpp](../tests/unit/protocol/http/v11/response_tests.cpp) - `large bodies serialize through an owned source` | Bodies of 2047/2048/2049 position-dependent bytes; correct inline/owned-source selection and complete exact bytes. |
| R-S01 | [tests/unit/common/byte_storage_tests.cpp](../tests/unit/common/byte_storage_tests.cpp) - `spilling preserves existing files` | Existing sentinel and spill contents checked; owned spill removal observed before fixture cleanup. This does not force a generated-name collision (Q03). |
| R-S02 | [tests/unit/common/byte_storage_tests.cpp](../tests/unit/common/byte_storage_tests.cpp) - `truncated spill files fail the reader` | Exactly one spill before truncation, persistent read error, and file removal before fixture cleanup. |
| R-E01 | [tests/unit/transport/server/executor_tests.cpp](../tests/unit/transport/server/executor_tests.cpp) - `executor resumes a bounded batch exactly once` | 63/64/65 handles around batch size 64; every handle counter exactly one. |
| R-E02 | [tests/unit/transport/server/executor_tests.cpp](../tests/unit/transport/server/executor_tests.cpp) - `executor supports concurrent producers and consumers` | Four producers and four consumers really overlap; bounded handshake and per-handle exact counts. |
| R-F01 | [tests/unit/common/date_server_tests.cpp](../tests/unit/common/date_server_tests.cpp) - `concurrent serialization preserves HTTP dates` | Workers meet a barrier before the timed window; each thread serializes at least once; full calendar/weekday date checks. |
| R-F02 | [tests/unit/common/date_server_tests.cpp](../tests/unit/common/date_server_tests.cpp) - `date server remains active until its last owner stops` | RAII owner cleanup, valid initial and updated date, bounded update window. |
| R-F03 | [tests/unit/common/date_server_tests.cpp](../tests/unit/common/date_server_tests.cpp) - `concurrent date server owners preserve lifecycle and dates` | Existing anchor scenario retained; each worker completes its expected 100 operations; date components and weekday valid. |
| R-T01 | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) - `tcpip streams response sources across send boundaries` | Position-dependent binary streamed payload across 8192/65536 boundaries; all bytes compared. |
| R-T02 | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) - `tcpip sends prefixes larger than its bounded send buffer` | Nonuniform oversized prefix; entire prefix compared. |
| R-T03 | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) - `tcpip completes a streamed response before its successor` | Distinct streamed bodies; exact concatenated output and no extra bytes. |
| R-T04 | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) - `tcpip serves another client while a large response is blocked` | OPEN: fixture lifetime improved, but pending send/backpressure at the observation point is not proven. Serialized count is insufficient. |
| R-T05 | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) - `tcpip recovers after a client resets a large response` | 8193 verified prefix bytes received before reset; server remains usable and teardown is safe. |
| R-T06 | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) - `tcpip stops idle blocked and deferred clients exactly once` | Mixed, idle-only, blocked-only and deferred-only scenarios; exact connected/disconnected totals after stop/join. Pending-send proof remains Q04. |
| R-I01 | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) - `HTTP/1.1 emits interim and automatic response behavior` | One original registration replaced by four named cases with exact response bodies/framing and independently checked dates. |

<a name="r-i01-and-o01-four-separated-existing-behaviors"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-r-i01-and-o01-four-separated-existing-behaviors-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-r-i01-and-o01-four-separated-existing-behaviors-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-r-i01-and-o01-four-separated-existing-behaviors-dark.svg">
    <img src="../resources/docs/test-audit/h3-r-i01-and-o01-four-separated-existing-behaviors.svg" alt="R-I01 and O01: four separated existing behaviors">
  </picture>
</h3>

These replace `HTTP/1.1 emits interim and automatic response behavior` in
[tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp).
All four passed. Registration delta: +3; new functional-case delta: zero.

| Behavior | Final registered name |
| --- | --- |
| 100 Continue and echo | `HTTP/1.1 echoes a body after 100 Continue` |
| Invalid If-Modified-Since | `HTTP/1.1 ignores invalid If-Modified-Since` |
| Invalid If-Unmodified-Since | `HTTP/1.1 ignores invalid If-Unmodified-Since` |
| 405 and Allow | `HTTP/1.1 reports the allowed method` |

<a name="5-fragmentation-and-boundary-measurements"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-5-fragmentation-and-boundary-measurements-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-5-fragmentation-and-boundary-measurements-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-5-fragmentation-and-boundary-measurements-dark.svg">
    <img src="../resources/docs/test-audit/h2-5-fragmentation-and-boundary-measurements.svg" alt="5. Fragmentation and boundary measurements">
  </picture>
</h2>

The following numbers are derived from the actual C++ input bytes and loop
bounds and were exercised by passing suites. A malformed input of n bytes
gets n+1 two-fragment split positions, n+1 selected three-fragment layouts
(the second cut follows the first by one byte), and one bytewise run.
These are scheduled layouts, including empty boundary fragments, not
distinct registered cases or all possible three-fragment partitions.
Feeding stops at the first terminal rejection; no request may be produced.

| ID | Malformed inputs | Two-fragment positions | Total scheduled layouts |
| --- | --- | --- | --- |
| R-D02 | 15 | 561 | 1137 |
| R-D04 | 9 | 458 | 925 |
| R-D05 | 1 | 54 | 109 |
| R-D06 | 4 | 118 | 240 |
| R-D07 | 6 | 365 | 736 |
| R-D08 | 1 | 82 | 165 |
| R-D09 | 2 | 146 | 294 |
| R-D10 | 5 | 190 | 385 |
| Total | 43 | 1974 | 3991 |

The absolute-form/Host mismatch in R-D04 stays as one unsplit compatibility
expectation pending Q01 and is excluded from that fragmentation table.
R-D03 separately has 23 inputs, 258 prefix positions and 23 bytewise runs;
its literal invalid-byte thresholds distinguish an incomplete valid prefix
from an already impossible request line. N-D05/N-D06 use complete pipeline
inputs of 111/126 bytes and execute 112/127 two-fragment split positions.

For the chunked limits, the extension budget includes the semicolon and
extension token, but excludes the chunk size and framing. The trailer budget
includes `X: `, its value and the final `CRLFCRLF`, but excludes `0 CRLF`.
Below/at/above cases are complete, not deliberately unfinished prefixes.

| Budget | Budget lengths | Full wire lengths | R-B05 split positions |
| --- | --- | --- | --- |
| extension | 1023/1024/1025 | 1034/1035/1036 | 3108 |
| trailers | 4095/4096/4097 | 4098/4099/4100 | 12300 |

R-B05 executes 15408 split positions. R-B06 executes the same six inputs at
two output capacities (12 reader evaluations). Below/at the limit succeeds;
above it reports the specific persistent extension/trailer limit error.
N-D03 checks complete heads of 5119/5120 bytes; N-D04 checks 127/128 query
pairs. The unimplemented overflow policy is explicitly Q02.

Other finite boundaries are 2047/2048/2049 response bytes (R-O02),
65534/65535/65536 stored raw or encoded chunked bytes (N-D08/N-D09 and HTTP
spill scenarios), 63/64/65 executor handles (R-E01), and 28 route inputs
through three entry points (84 route evaluations).

<a name="6-harness-cleanup-and-mutation-evidence"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-6-harness-cleanup-and-mutation-evidence-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-6-harness-cleanup-and-mutation-evidence-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-6-harness-cleanup-and-mutation-evidence-dark.svg">
    <img src="../resources/docs/test-audit/h2-6-harness-cleanup-and-mutation-evidence.svg" alt="6. Harness, cleanup and mutation evidence">
  </picture>
</h2>

Both existing runners now print and flush the active file/name/line before
invocation, report assertion expression/location and owned row context, catch
standard and unknown exceptions per case, continue after those failures, and
return 0 for success, 1 for failed cases, or 2 for invalid arguments/no match.
`--list` is read-only; `--file` is a normalized path substring, `--name` and
`--exclude` match an exact name. The existing fatal-return assertion macros
and test registration style are retained.

The two private probe executables and `tests/verify_test_helper.cmake`
exercise the following ten named checks per runner. All passed in all ten
configurations. Their intentionally false assertions and thrown exceptions
are expected subprocess outcomes, not suppressed functional-suite failures.

| ID | Auxiliary check | Observed result |
| --- | --- | --- |
| H01 | List names/files without executing bodies. | Exit 0; body marker absent. |
| H02 | Filter by file and exact name. | Only the selected passing body runs. |
| H03 | No matching case. | Exit 2 and No tests matched. |
| H04 | False assertion. | Exit 1; exact expression and test_helper_probe.cpp:line reported. |
| H05 | Standard exception. | Exception message reported; later case runs in the combined probe. |
| H06 | Unknown exception. | Unknown exception reported; later case runs in the combined probe. |
| H07 | Blocked case is identified before waiting. | Flushed active-case output captured before parent timeout. |
| H08 | Success/failure/argument result codes. | 0 for passing selection, 1 for failure/exception, 2 for invalid argument. |
| H09 | Row context. | row 7: input=invalid appears in failure output. |
| H10 | Parent timeout is a failure outcome. | The deliberate 30-second blocker is killed after one second and reported as timeout. |

One additional integration probe forces assertion failure with an active TCP
connection; destruction joins the server before captured state disappears,
then exact connection/disconnection counts and a cleanup marker are checked.
This is the 21st auxiliary check. Probe subprocess deadlines are three
seconds except the one-second blocker. CTest registers the unit suite, its
probe verifier, the integration suite and its probe verifier, each with
TIMEOUT 120. The longest final functional entry was 36.18 seconds under
TSan, leaving over three times that observation as timeout margin.

Before the harness change, an isolated old-runner probe ignored listing and
an uncaught exception terminated execution. The before-build/test evidence
is in `out/build/test-audit-gcc-debug/probe-before-*.log`. Afterwards the
meta-checks passed on Windows and Linux. Native crashes/termination still
end the affected aggregate process; each functional case is not launched in
a separate process. Filters and flushed identity permit focused reruns.

<a name="suspended-failure-path"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-suspended-failure-path-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-suspended-failure-path-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-suspended-failure-path-dark.svg">
    <img src="../resources/docs/test-audit/h3-suspended-failure-path.svg" alt="Suspended failure path">
  </picture>
</h3>

An isolated copy of `tcpip preserves response order for pipelined deferred
requests` was made to fail immediately after suspension. With only declaration
ordering repaired, ASan reported **10184 leaked bytes in 11 allocations**.
The coroutine retained request/response state because its external signal
was never resumed after the assertion return. A private `deferred_cleanup`
guard now covers all ten deferred TCP fixtures. Its destruction follows
server stop/join and resumes outstanding signals while captured state is
still alive. The same forced assertion still exits 1, but ASan reports no
leak after that adjustment. New HTTP suspension fixtures use their private
latched signal and stop callbacks to release pending work on cancellation.

Evidence: `out/build/test-audit-fixture-probe/before.log`, `after.log`,
`build.log` and `after-build.log`. This experiment remains outside the
functional case count. The full ASan, UBSan and TSan suites also passed.

<a name="four-isolated-mutations"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-four-isolated-mutations-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-four-isolated-mutations-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-four-isolated-mutations-dark.svg">
    <img src="../resources/docs/test-audit/h3-four-isolated-mutations.svg" alt="Four isolated mutations">
  </picture>
</h3>

Only copied headers under `out/build/test-audit-mutations/M01..M04/include/`
were mutated. Each corresponding test failed with exit 1 for the intended
assertion; the copied headers were then restored from production. The
production tree was never edited. No experiment resumed a completed handle
twice. The retained mutant binaries/logs are diagnostic artifacts.

| ID | Mutation | Detecting case | Failure observed |
| --- | --- | --- | --- |
| M01 | Omit Accept dispatcher registration. | N-H01-N: decoder rejects invalid Accept. | Expected invalid-source status; row context identifies Accept: text/. |
| M02 | Skip copying the memory prefix when opening a spill. | N-S03: crossing the threshold preserves the memory prefix. | Expected ten-byte read fails. |
| M03 | Raw reader consumes one successor byte after body completion. | R-B01: reads exactly content length for every output size. | Expected four remaining NEXT bytes fails. |
| M04 | Skip the first queued continuation. | R-E01: executor resumes a bounded batch exactly once. | Per-handle exact-one counter fails. |

Each directory contains `build.log` and `test.log`. Four caught mutations
provide targeted evidence of detection; this is not a repository-wide
mutation score or a statistical quality estimate.

<a name="7-executed-configurations-and-results"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-7-executed-configurations-and-results-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-7-executed-configurations-and-results-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-7-executed-configurations-and-results-dark.svg">
    <img src="../resources/docs/test-audit/h2-7-executed-configurations-and-results.svg" alt="7. Executed configurations and results">
  </picture>
</h2>

Baseline suites were freshly built at the stated revision with strict
warnings and examples disabled: GCC Debug passed 401/39 cases in 8.24/5.29
seconds (13.61 total CTest seconds); MSVC Debug passed in 7.40/2.45 seconds
(9.92 total). Their baseline-build.log/baseline-test.log files remain under
`out/build/test-audit-gcc-debug/` and `test-audit-msvc-debug/`.

Environment: Windows MSVC 19.51.36256 and CMake 4.2.1; WSL2 Ubuntu 24.04.3,
Linux 6.6.87.2-microsoft-standard-WSL2, GCC 13.3.0, Clang 18.1.3 and CMake
3.28.3. Minimum-version validation used the official CMake 3.20.6 Linux
archive after checking its published SHA-256 manifest. All builds used
Ninja and DOBA_ENABLE_STRICT_WARNINGS=ON. Existing baseline Debug build
directories retain examples=OFF; the fresh Release, Clang, sanitizer and
minimum-CMake directories build examples with the project defaults.

| Build suffix | Unit (556), seconds | Integration (71), seconds | CTest result | Total seconds |
| --- | --- | --- | --- | --- |
| gcc-debug | 20.07 | 33.49 | 4/4 passed | 55.70 |
| gcc-release | 15.47 | 33.47 | 4/4 passed | 51.16 |
| clang-debug | 22.01 | 33.47 | 4/4 passed | 57.71 |
| clang-release | 16.50 | 33.48 | 4/4 passed | 52.22 |
| msvc-debug | 18.56 | 34.09 | 4/4 passed | 55.06 |
| msvc-release | 14.64 | 33.39 | 4/4 passed | 50.44 |
| clang-asan | 21.13 | 33.68 | 4/4 passed | 57.26 |
| clang-ubsan | 22.03 | 33.53 | 4/4 passed | 57.87 |
| clang-tsan | 36.18 | 33.82 | 4/4 passed | 72.56 |
| cmake-3.20 | 18.03 | 33.51 | 4/4 passed | 53.79 |

Each build directory is `out/build/test-audit-<suffix>/`. Final complete
runs are recorded in `verified-build.log` and `verified-test.log`; detailed
per-case runner output is in `Testing/Temporary/LastTest.log`. Each of the
four CTest entries passed with zero failed functional cases. Build/test
commands returned zero. Durations include contention between some local
configurations and are observations, not performance baselines.

Sanitizer builds used Debug, Clang, `-fno-omit-frame-pointer`, and respectively
`-fsanitize=address`, `-fsanitize=undefined`, or `-fsanitize=thread`. Runtime
options were `ASAN_OPTIONS=halt_on_error=1:detect_leaks=1`,
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`, and
`TSAN_OPTIONS=halt_on_error=1`. No suppression was added.

The GCC Release installation was consumed by independent Debug and Release
`find_package` builds and a Release `add_subdirectory` build. All three built
and their existing consumer executables exited 0. The source consumer also
verified that internal tests/examples were not added to its parent build.
Evidence directories: `out/build/test-audit-package-debug/`,
`test-audit-package-release/`, and `test-audit-subdirectory/` (`build.log`,
`run.log`). Configure/install logs are in `test-audit-gcc-release/`.
Packaging sources, dependency interfaces and the CI workflow are unchanged.

During drafting, the HTTP rejection tests initially required a
Connection: close response field. Inspection showed that the existing
server sends 400 with body Invalid request content! and actually closes the
socket, without that optional field. The new oracle was corrected to those
exact bytes, EOF and zero handler calls. The refused-connect draft budget
was also raised from 300 ms to the existing three-second client default
because Windows refusal can arrive later; the test still requires a socket
error, never a timeout. No existing assertion was weakened.

Final style review aligned two new class comment tags and removed empty
string concatenations around an unchanged ETag input. The affected targets
were rebuilt and the three ETag cases rerun under GCC/MSVC Debug; those
focused results are in `style-build.log` and `style-test.log`. These final
source edits do not change the inputs or expectations of the matrix above.

<a name="8-open-decisions-and-localized-follow-up-plans"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-8-open-decisions-and-localized-follow-up-plans-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-8-open-decisions-and-localized-follow-up-plans-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-8-open-decisions-and-localized-follow-up-plans-dark.svg">
    <img src="../resources/docs/test-audit/h2-8-open-decisions-and-localized-follow-up-plans.svg" alt="8. Open decisions and localized follow-up plans">
  </picture>
</h2>

None of Q01..Q08 is marked complete. Their production/API changes were
excluded from the approved base scope. The following proposals require a
separate concrete approval before implementation; the passing case count
does not resolve them.

<a name="q01-absolute-form-authority-and-host"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q01-absolute-form-authority-and-host-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q01-absolute-form-authority-and-host-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q01-absolute-form-authority-and-host-dark.svg">
    <img src="../resources/docs/test-audit/h3-q01-absolute-form-authority-and-host.svg" alt="Q01: absolute-form authority and Host">
  </picture>
</h3>

**Confirmed.** `GET http://a/ HTTP/1.1` with `Host: b` is rejected with
invalid-source, reason zero, and no request. The equality check in
[include/protocol/http/v11/headers/rules/routing.h](../include/protocol/http/v11/headers/rules/routing.h)
applies to all target authorities. For origin-server absolute-form processing,
[RFC 9112 S3.2.2](https://www.rfc-editor.org/rfc/rfc9112.html#section-3.2.2)
requires using the target authority in preference to the received Host.

**Proposed follow-up.** Correct only the absolute-form routing branch, retain
syntax/Host multiplicity validation, and define how effective authority is
exposed while preserving the raw header value and public signatures. Keep
authority-form/CONNECT behavior separate. Expected files are routing.h,
`include/protocol/http/v11/decoder.h`,
`tests/unit/protocol/http/v11/headers/rules/routing_tests.cpp`,
`tests/unit/protocol/http/v11/decoder_tests.cpp`, and
`tests/integration/protocol/http/v11/server_tests.cpp`. Add failing regressions
for different hosts, different explicit ports, default-port equivalence and
effective authority/raw-header preservation. Run rule/decoder tests and real
socket cases on IOCP/epoll. The existing get_host versus target-authority
accessor contract must be settled in that plan; no accessor change was made
here.

<a name="q02-query-overflow-and-a-full-incomplete-head"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q02-query-overflow-and-a-full-incomplete-head-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q02-query-overflow-and-a-full-incomplete-head-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q02-query-overflow-and-a-full-incomplete-head-dark.svg">
    <img src="../resources/docs/test-audit/h3-q02-query-overflow-and-a-full-incomplete-head.svg" alt="Q02: query overflow and a full incomplete head">
  </picture>
</h3>

**Confirmed.** A request with 129 unique query pairs succeeds with only
128 visible pairs; k128 is absent. The splitter intentionally stops at its
output capacity, whereas limits.h describes rejection. A complete 5121-byte
head first accumulates 5120 bytes and reports MoreBytesNeeded; the remaining
byte then accumulates zero and the decoder again reports MoreBytesNeeded.
This proves lack of progress at the decoder boundary, not a measured
end-to-end permanent server hang or an existing 431 response.

**Proposed follow-up.** Resolve the rejection/truncation contract explicitly
(rejection is the documented limits.h intent), detect overflow and a full
head that cannot complete, and return a terminal existing error rather than
MoreBytesNeeded with no capacity. Expected files are
`include/protocol/http/v11/decoder.h`, `limits.h` in the same directory,
`tests/unit/protocol/http/v11/decoder_tests.cpp`, `limits_tests.cpp`, and
`tests/integration/protocol/http/v11/server_tests.cpp`. Inspect the contract
of `include/protocol/http/common/helpers.h`; if changing its behavior or
comments becomes necessary, include it and its focused tests explicitly in
the approved follow-up. Preserve public signatures. Test 127/128/129 pairs,
empty pair separators, complete 5119/5120/5121-byte heads, every relevant
full-buffer transition and socket closure. Select the reason/status before
implementation; no new 431 policy was invented here.

<a name="q03-forced-spill-name-collision"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q03-forced-spill-name-collision-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q03-forced-spill-name-collision-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q03-forced-spill-name-collision-dark.svg">
    <img src="../resources/docs/test-audit/h3-q03-forced-spill-name-collision.svg" alt="Q03: forced spill-name collision">
  </picture>
</h3>

**Open.** N-S15 proves eight concurrent owners get independent files, and
R-S01 proves preservation of an existing sentinel. Neither forces an actual
candidate filename collision. Linux uses mkstemp; Windows uses a private
PID/tick/counter candidate with CREATE_NEW retries.

**Proposed follow-up.** Design a narrowly scoped deterministic test seam for
occupied candidate names, then verify sentinel preservation, retry, failure
classification and owner cleanup. Expected areas are
`include/common/byte_storage_helpers_linux.h`,
`include/common/byte_storage_helpers_windows.h` and
`tests/unit/common/byte_storage_tests.cpp`. Any necessary seam/build change
must be specified and approved first. No public naming option or generic
injection framework is proposed by this implementation.

<a name="q04-proven-pending-output-and-blocked-send_all"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q04-proven-pending-output-and-blocked-send_all-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q04-proven-pending-output-and-blocked-send_all-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q04-proven-pending-output-and-blocked-send_all-dark.svg">
    <img src="../resources/docs/test-audit/h3-q04-proven-pending-output-and-blocked-send_all.svg" alt="Q04: proven pending output and blocked send_all">
  </picture>
</h3>

**Open.** R-T04 still does not establish that the large response has bytes
pending when another client is served. The serialization counter only proves
response preparation. Position-dependent bytes, large payloads and sleeps
are not proof of socket backpressure. A permanently blocked send_all path
also has no deterministic regression yet.

**Proposed follow-up.** Configure receive buffers before connect and provide
an observable pending-send phase for a non-reading local peer. Verify that
client two completes while client one still has pending output, and that
send_all times out on its one operation budget. Expected test files are
`tests/integration/tcpip_client.h`, `tcpip_client_tests.cpp`, and
`tests/integration/transport/server/tcpip_tests.cpp`. If kernel/socket state
cannot provide deterministic evidence, propose the exact private observation
point in `include/transport/server/tcpip_linux.h` and `tcpip_windows.h` before
editing production. R-T04 remains outside the completed reinforcement count.

<a name="q05-reserve-then-bind-port-race"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q05-reserve-then-bind-port-race-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q05-reserve-then-bind-port-race-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q05-reserve-then-bind-port-race-dark.svg">
    <img src="../resources/docs/test-audit/h3-q05-reserve-then-bind-port-race.svg" alt="Q05: reserve-then-bind port race">
  </picture>
</h3>

**Open.** The test client closes its port-zero reservation before the real
server binds. Local passing runs are not evidence that this race is removed.

**Proposed follow-up.** Reproduce occupation between reservation and startup,
and assert address-in-use distinctly. Decide between adopting a reserved
socket and querying the actual bound port, then specify any needed API
change. Expected areas are `tests/integration/tcpip_client.h`,
`tests/integration/transport/server/tcpip_tests.cpp`,
`include/transport/server/tcpip.h` and both platform backends. No arbitrary
retry or public transport method was added without that design/approval.

<a name="q06-timeout-of-an-actually-pending-connect"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q06-timeout-of-an-actually-pending-connect-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q06-timeout-of-an-actually-pending-connect-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q06-timeout-of-an-actually-pending-connect-dark.svg">
    <img src="../resources/docs/test-audit/h3-q06-timeout-of-an-actually-pending-connect.svg" alt="Q06: timeout of an actually pending connect">
  </picture>
</h3>

**Open.** N-C01 exercises refusal, not a connection that remains pending.
A successful timed refusal does not validate expiration of pending connect.

**Proposed follow-up.** Demonstrate a deterministic local pending-connect
mechanism on both platforms, then add a focused timeout/error/deadline case
in `tests/integration/tcpip_client_tests.cpp`. Only adjust tcpip_client.h if
the reproducer exposes a defect. A backlog-saturation technique needs
evidence of its portability; external blackhole hosts/Internet timing are
excluded. This mechanism is still unproven, not silently skipped.

<a name="q07-public-resource-limit-contract"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q07-public-resource-limit-contract-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q07-public-resource-limit-contract-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q07-public-resource-limit-contract-dark.svg">
    <img src="../resources/docs/test-audit/h3-q07-public-resource-limit-contract.svg" alt="Q07: public resource-limit contract">
  </picture>
</h3>

**Open.** BACKLOG C2 still owns configurable limits/defaults. The finite
internal boundaries exercised here are not that public policy.

**Proposed follow-up.** First agree the public API, defaults, early rejection
and encoded/decoded chunked accounting in C2. Then list its implementation
files and add zero/L-1/L/L+1 cases for each selected limit. No speculative
configuration or API was introduced in this test expansion.

<a name="q08-te-with-connection-te"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h3-q08-te-with-connection-te-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h3-q08-te-with-connection-te-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h3-q08-te-with-connection-te-dark.svg">
    <img src="../resources/docs/test-audit/h3-q08-te-with-connection-te.svg" alt="Q08: TE with Connection: TE">
  </picture>
</h3>

**Confirmed.** `TE: trailers` together with `Connection: TE` is rejected with
invalid-source, reason zero, and no request. directives.h forbids the te
connection option. [RFC 9110 S10.1.4](https://www.rfc-editor.org/rfc/rfc9110.html#section-10.1.4)
requires a TE sender to include that connection option.

**Proposed follow-up.** Remove the specific inappropriate te prohibition,
retain all unrelated directive behavior, and update the rule documentation
and focused expectations. Expected files are
`include/protocol/http/v11/headers/rules/directives.h`,
`tests/unit/protocol/http/v11/headers/rules/directives_tests.cpp` and
`tests/unit/protocol/http/v11/decoder_tests.cpp`. Add Q08-M01-P for
TE: trailers plus Connection: TE and Q08-M01-N for TE: gzip;q=1.001 in the
same conformant context, including the six name/OWS variants. The negative
must fail because of its TE value, not because its context is rejected
first. These two candidates are not present in the 184 new cases.

The focused Q01/Q02/Q08 probe and captured output are in
`out/build/test-audit-decisions/probe.cpp` and `result.log`. Numeric status
values in that log mean success=0, invalid-source=1, more-bytes-needed=2.
Those diagnostic reproductions are not accepted regression expectations
for the proposed corrected behavior. They remain deliberately outside the
functional count and do not certify the rest of the protocol.

<a name="9-changed-files-style-and-remaining-limits"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/test-audit/h2-9-changed-files-style-and-remaining-limits-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/test-audit/h2-9-changed-files-style-and-remaining-limits-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/test-audit/h2-9-changed-files-style-and-remaining-limits-dark.svg">
    <img src="../resources/docs/test-audit/h2-9-changed-files-style-and-remaining-limits.svg" alt="9. Changed files, style and remaining limits">
  </picture>
</h2>

The implementation changes the existing unit/integration runners and their
CMakeLists.txt files, the integration client, the 52 R-H header test files,
and the test files individually linked in sections 3/4. New private files:

- [tests/unit/test_helper_probe.cpp](../tests/unit/test_helper_probe.cpp)
- [tests/integration/test_helper_probe.cpp](../tests/integration/test_helper_probe.cpp)
- [tests/verify_test_helper.cmake](../tests/verify_test_helper.cmake)
- [tests/integration/tcpip_client_tests.cpp](../tests/integration/tcpip_client_tests.cpp)
- [tests/integration/protocol/http/v11/http_test_helper.h](../tests/integration/protocol/http/v11/http_test_helper.h)

Documentation changes are this report and [docs/BACKLOG.md](../docs/BACKLOG.md).
New C++ headers were compared with the closest existing counterpart: doba
logo, copyright and license text match exactly. Test names, DOBA_TEST and
DOBA_EXPECT macros, comment boxes, indentation and local data-table patterns
are preserved. All modified/new text was checked as ASCII, UTF-8 without
BOM, CRLF with final newline; tracked-file EOL status and git diff --check
were checked separately. Existing unrelated formatting and user files were
left unchanged. A separate read-only EOL inventory also found existing mixed
endings in `tests/unit/common/task_tests.cpp` and
`tests/unit/protocol/http/v11/request_tests.cpp`; neither file was modified.

The HTTP test oracle is private and intentionally limited to the response
forms emitted by these scenarios: literal status lines, the existing field
spelling, Content-Length framing and no-body statuses/HEAD. It reads exactly
one response, leaving the successor in the socket. It rejects duplicate
framing/date fields and validates calendar/weekday dates without using the
production decoder as an oracle. It is not a general HTTP response parser,
a chunked-output validator or a replacement protocol implementation.

The base expansion is complete; Q01..Q08 and R-T04 remain visible follow-up
work. Finite concurrency cases and clean sanitizer runs do not replace
fuzzing, soak tests, deterministic scheduling campaigns or exhaustive RFC
review. No new testing dependency, public production abstraction, compiler
standard, warning policy, sanitizer configuration or CI job was needed.
Local build artifacts and logs are retained under out/ and are not project
source changes. The existing dependency-free testing approach is preserved.
