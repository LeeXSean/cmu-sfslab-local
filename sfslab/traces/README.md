# Trace Catalog

The official course infrastructure runs SFS tests as Lua/Lanes traces. This
offline package does not execute these files yet; the runnable local grader is
`local/test-sfs.c`.

These traces document the intended shape of the A/B/C test suites and provide
a clean target for a future Lua runner.

## Categories

- `A/`: single-threaded feature tests.
- `B/`: sequential versions of more complex operations.
- `C/`: concurrent versions of related operations.
- `MANIFEST.tsv`: trace id, category, file path, and purpose.

The helper names used below (`disk.open`, `disk.write`, `check`, `lanes.gen`,
etc.) follow the course-style trace vocabulary, but this repository does not
ship a Lua binding yet.

Run `make trace-check` from the handout root to syntax-check these files when
`luac` is installed. Run `make trace-smoke` to check the starter-safe traces,
or `make trace-run` to execute the full catalog through the local Lua binding.

Run `make manifest-check` to compare `MANIFEST.tsv` with the local autograder's
current trace list.
