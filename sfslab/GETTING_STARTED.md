# Getting Started

This local package is meant for self-study. Work on your own `sfs-disk.c`; the
test tools are here to show progress, not to replace the assignment.

## Suggested Order

1. Run `make check`.
   This confirms the starter package and local tools are healthy.

2. Read the `sfs_getpos`, `sfs_seek`, and `sfs_rename` comments in `sfs-api.h`.
   These are the intentionally incomplete starter functions.

3. Work through Category A first.
   Run `./test-sfs -v` or `make json` to see the next visible failure. The
   starter is expected to fail A02, A03, and A04 until those functions are
   implemented.

4. Move to Category B after A is stable.
   These tests focus on sequential edge cases such as multi-block reads,
   invalid file descriptors, open-file lifecycle behavior, and rename/list
   interactions.

5. Move to Category C after A and B are stable.
   Category C is the normal concurrent correctness signal. It is part of the
   22-point local score. Race detection and repeated stress are separate
   follow-up signals; see `local/SCORING.md`.

6. Use `make stress` only as a diagnostic.
   Stress checks are intentionally outside the 22-point score. They are useful
   after you start working on concurrency, but they are not the first thing to
   chase.

7. Tune performance last.
   The performance benchmark only runs after correctness reaches 12/12. Run
   `make baseline` on your machine before treating performance numbers as
   meaningful.

## Reading Scores

`make json` prints the local 22-point score. `make report-json` also includes
Lua trace coverage and stress diagnostics, but those sections are not added to
the 22-point score.
