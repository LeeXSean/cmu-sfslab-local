# Local Scoring

The local score is a self-study signal, not an Autolab prediction.

## Current Weights

- Feature tests: 5 points
- Sequential correctness: 4 points
- Concurrent correctness: 3 points
- Performance: 10 points
- Style: manual, up to 4 points

The local 22-point score is intentionally frozen at these weights. New checks
should be reported as Lua coverage or stress diagnostics unless the scoring
contract is deliberately revised.

Performance only runs after all correctness traces pass.

Correctness traces also run `sfs-fsck` after unmounting their disk image. A
trace fails if the visible API result looks right but the filesystem structure
is corrupt, and the trace failure includes the first captured checker
diagnostic.

Run `make trace-list` to print the current local autograder items.
Run `make json` to print a machine-readable score summary.
Run `make trace-json` to print machine-readable Lua trace coverage. Lua trace
coverage is useful for comparing against the official trace style, but it is
reported separately and is not added to the 22-point local score.
From the repository root, `make trace-run`, `make trace-smoke`, and
`make trace-json` fall back to Docker when local Lua development dependencies
are missing.
Run `make report-json` to print the local score, Lua trace coverage, and stress
diagnostics in one JSON document. Only `local_autograder.result.total` is the
graded 22-point score; Lua trace coverage and stress diagnostics are separate
signals.
Run `make starter-safe` to check that the packaged starter and starter-safe
trace subset still run. This is the package health check; it is not a score.
Run `make stress` to repeat the C concurrency traces and extra stress-only
diagnostics without running the full scoreboard; override the loop count with
`STRESS_RUNS=N`. A failure here means the implementation is not stable under
repeated concurrent schedules. `stress` is for testing an implementation after
you start fixing concurrency, not for proving that the starter is complete. The
C concurrency and stress-only diagnostics use isolated disk images so one
trace's cleanup path does not contaminate the next trace.

## How To Read The Score

Prioritize correctness first. A fast implementation with failing A/B/C traces
is not useful. ThreadSanitizer runs after all normal A/B/C correctness traces
pass; the JSON report records `tsan_status` as `clean`, `race_detected`,
`trace_failed`, `timeout`, `skipped`, or `unavailable`. A TSan race, TSan trace
failure, or TSan timeout sets the C score to zero. If a normal correctness
trace already fails, TSan is skipped and the normal per-trace score is
reported. Use the performance score only after the implementation is stable
under the sequential tests, concurrent traces, and ThreadSanitizer.

The performance score compares your implementation to the local
`sfs-baseline-ref.c` coarse-lock implementation on the same machine. It is
useful for tracking progress on one setup, but it is still affected by Docker,
WSL, filesystem latency, scheduler noise, and CPU load.
