# Local Scoring

The local score is a self-study signal, not an Autolab prediction.

## Current Weights

- Feature tests: 5 points
- Sequential correctness: 4 points
- Concurrent correctness: 3 points
- Performance: 10 points
- Style: manual, up to 4 points

Performance only runs after all correctness traces pass.

Run `make trace-list` to print the current local autograder items.
Run `make json` to print a machine-readable score summary.
Run `make trace-json` to print machine-readable Lua trace coverage. Lua trace
coverage is useful for comparing against the official trace style, but it is
reported separately and is not added to the 22-point local score.
Run `make report-json` to print both reports in one JSON document.

## How To Read The Score

Prioritize correctness first. A fast implementation with failing A/B/C traces
is not useful. ThreadSanitizer runs after all C traces pass; if it reports a
race, the C score is set to zero. Use the performance score only after the
implementation is stable under the sequential tests, concurrent traces, and
ThreadSanitizer.

The performance score compares your implementation to the local
`sfs-baseline-ref.c` coarse-lock implementation on the same machine. It is
useful for tracking progress on one setup, but it is still affected by Docker,
WSL, filesystem latency, scheduler noise, and CPU load.

## Future Work

- Add more A/B/C traces before adding more performance thresholds.
- Expand the Lua trace catalog now that `traces/` can run locally.
- Use the trace manifest's starter metadata to keep smoke checks aligned with
  the Lua catalog.
- Keep the local score separate from any official course score.
