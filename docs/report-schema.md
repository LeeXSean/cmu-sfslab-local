# Report JSON Schema Notes

`make report-json` emits one JSON document that combines the graded local
autograder score with diagnostic trace and stress sections.

The command exits 0 when report generation succeeds, even if the starter or a
student implementation fails graded traces. Use `make report-json-strict` when
the process should fail on a non-passing graded score.

## Top-Level Shape

```json
{
  "local_autograder": {
    "exit_status": 1,
    "result": {}
  },
  "official_style_traces": {
    "available": false,
    "exit_status": 77,
    "result": null
  },
  "stress_diagnostics": {
    "available": true,
    "exit_status": 1,
    "graded": false,
    "result": {}
  },
  "combined": {
    "graded_total_uses": "local_autograder.result.total",
    "trace_coverage_is_diagnostic": true,
    "stress_diagnostics_is_diagnostic": true,
    "graded_passed": false,
    "diagnostics_passed": false
  }
}
```

## Fields

`local_autograder.exit_status` is the exit status from `./test-sfs --json`.
For the packaged starter this is nonzero because the starter is intentionally
incomplete.

`local_autograder.result` is the graded 22-point local score object:

- `correctness`: score and max for A/B/C correctness, max 12.
- `categories.A`: feature tests, max 5.
- `categories.B`: sequential correctness, max 4.
- `categories.C`: concurrent correctness, max 3, plus ThreadSanitizer status.
- `performance`: performance score, max 10, only runs after correctness passes.
- `total`: the graded local score, max 22.
- `style`: manual style-review placeholder, max 4, not auto-graded.
- `passed`: true only when the graded correctness gate passes.

`official_style_traces` reports Lua trace coverage in a course-trace-like
style. This section is diagnostic only and does not change the 22-point score.
When local Lua development headers are unavailable, `available` is false and
`exit_status` is 77.

`stress_diagnostics` reports schedule-sensitive stress checks. This section is
diagnostic only and intentionally has `"graded": false`.

`combined.graded_total_uses` identifies the only graded score path:
`local_autograder.result.total`.

`combined.graded_passed` reflects the local autograder exit status.

`combined.diagnostics_passed` reflects diagnostic sections. Lua trace
unavailability with exit status 77 is not treated as a diagnostic failure
because Docker fallback targets can still run those traces separately.

## Starter Expectations

On a clean packaged starter:

- `make check` should pass.
- `make report-json` should emit parseable JSON and exit 0.
- `local_autograder.result.total.score` should be below 22.
- `combined.graded_passed` should be false.
- `make report-json-strict` should exit nonzero.

That starter status is a packaging health signal, not a completed lab solution.
