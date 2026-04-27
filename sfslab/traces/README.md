# Trace Catalog

The official course infrastructure runs SFS tests as Lua/Lanes traces. This
offline package ships a local Lua binding so these fixtures can run without
CMU infrastructure. The scored local grader is still `local/test-sfs.c`.

These traces document the intended shape of the A/B/C test suites and provide
an executable companion to the C autograder.

## Categories

- `A/`: single-threaded feature tests.
- `B/`: sequential versions of more complex operations.
- `C/`: concurrent versions of related operations.
- `MANIFEST.tsv`: trace id, category, file path, starter expectation, and
  purpose.

The helper names used below (`disk.open`, `disk.write`, `check`, `lanes.gen`,
etc.) follow the course-style trace vocabulary. The local binding implements
`lanes.gen` with pthreads and separate Lua states, so C traces make real
concurrent calls into the SFS API. The C autograder and ThreadSanitizer remain
the scored concurrency signal.

Run `make trace-check` from the handout root to syntax-check these files when
`luac` is installed. Run `make trace-smoke` to check the starter-safe traces,
or `make trace-run` to execute the full catalog through the local Lua binding.

Run `make manifest-check` to compare `MANIFEST.tsv` with the local autograder's
current trace list.
