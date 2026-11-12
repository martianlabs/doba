<a name="measured-release-baseline"></a>
<h1>
  <a href="../../resources/tests/qualification/baseline/h1-measured-release-baseline.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/baseline/h1-measured-release-baseline.svg" alt="Coverage snapshot">
  </a>
  <a href="../../resources/tests/qualification/baseline/h1-measured-release-baseline-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/baseline/h1-measured-release-baseline-dark.svg" alt="Coverage snapshot">
  </a>
</h1>

Here's the local reference from **2026-11-04**, with TLS and examples enabled.
Linux used Clang/LLVM 18.1.3 on WSL; Windows used Clang/LLVM 22.1.8 on Windows 11.

| Platform | Executable lines | Condition outcomes | Mapped / unmapped headers |
| --- | --- | --- | --- |
| Linux | 8169/8585 (95.15%) | 4730/5340 (88.58%) | 129 / 60 |
| Windows | 8436/8914 (94.64%) | 4788/5410 (88.50%) | 136 / 53 |

Lines and true/false outcomes come from `include/`, counted once across
executables. Unmapped headers stay outside the percentage. The two platforms
have different denominators. These numbers tell us where tests reached,
not how safe the code is.

| Module | Linux lines | Linux outcomes | Windows lines | Windows outcomes |
| --- | --- | --- | --- | --- |
| common | 89.87% | 81.96% | 88.67% | 78.95% |
| http-common | 97.06% | 93.83% | 97.06% | 93.88% |
| http-engine | 97.61% | 93.08% | 97.61% | 93.08% |
| http-messages | 91.87% | 83.33% | 91.87% | 83.33% |
| http-parser | 95.90% | 91.31% | 95.90% | 91.41% |
| network | no emitted mapping | no emitted mapping | 100.00% | 100.00% |
| public-contracts | 100.00% | 100.00% | 100.00% | 100.00% |
| static-files | 96.61% | 80.17% | 95.54% | 79.53% |
| tcp | 94.19% | 79.52% | 91.78% | 83.88% |
| tls | 99.21% | 79.78% | 99.21% | 79.78% |

<a name="evidence-and-gates"></a>
<h2>
  <a href="../../resources/tests/qualification/baseline/h2-evidence-and-gates.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/baseline/h2-evidence-and-gates.svg" alt="What ran">
  </a>
  <a href="../../resources/tests/qualification/baseline/h2-evidence-and-gates-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/baseline/h2-evidence-and-gates-dark.svg" alt="What ran">
  </a>
</h2>

- Full instrumented Release suites: Linux 21/21 groups, Windows 23/23.
- Twelve-round TLS-enabled campaigns: 888 actions under Linux TSan and
  696 under Windows ASan, passing after the timer-test fix described
  [here](README.md#reliability-finding-during-qualification).
- The corrected Windows processing group also passed 200 ASan and 200 UBSan runs.
  Winsock fault cases passed twelve UBSan repetitions.
- All 79 observed profiles on each OS were matched to their own executables.
  Zero-counter probes got no test credit.

The first measurement missed a Winsock startup failure branch. Three injected
error codes and cleanup/retry tests brought that module from 1/2 to 2/2 outcomes
(7/7 lines). That's the useful part of measuring: finding the next test to write.

[Linux](baselines/linux.json) and [Windows](baselines/windows.json) floors guard
against lost coverage. They aren't regenerated automatically to make CI pass.

The header lists were updated on 2026-11-10 after declarations were split and
transports moved. The totals and floors above were kept. Unmapped platform
alternatives are still listed. Raw exports, hashes and logs, including the
failed timer run, are in `out/reliability/2026-11-04`.
The [public coverage page](../../docs/coverage/README.md) keeps the release history.

<a name="next-coverage-driven-work"></a>
<h2>
  <a href="../../resources/tests/qualification/baseline/h2-next-coverage-driven-work.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/baseline/h2-next-coverage-driven-work.svg" alt="Where to look next">
  </a>
  <a href="../../resources/tests/qualification/baseline/h2-next-coverage-driven-work-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/baseline/h2-next-coverage-driven-work-dark.svg" alt="Where to look next">
  </a>
</h2>

Start with missed common ownership/error paths, TCP cleanup/scheduling and TLS
failures. Check why a path wasn't hit before excluding it. Wider native/TLS and
filesystem fuzzing, allocation failures and longer stress runs are still on the
[list](../security-audit.md#prioritized-remaining-work).
