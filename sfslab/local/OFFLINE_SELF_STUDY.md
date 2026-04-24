# Offline Self-Study Notes

This directory is arranged to feel close to the CMU SFS handout while still
being runnable without CMU infrastructure.

## What Matches the Handout Shape

- `sfs-disk.c` is the student file.
- `sfs-api.h`, `sfs-disk.h`, `sfs-support.c`, and `sfs-fsck.c` are provided
  support files.
- `.clang-format`, `.gitignore`, `.labname.mk`, and `README` mirror the kind of
  metadata present in a course handout.

## Local-Only Replacements

- `local/test-sfs.c` is a local C autograder. It replaces the course trace
  runner.
- `local/sfs-baseline-ref.c` is a coarse-lock reference used only for local
  performance calibration.
- `traces/` contains Lua-style trace fixtures for documentation and future
  runner work. They are not executed by the current local grader.
- The repository root `Dockerfile` provides a Linux toolchain for builds,
  ThreadSanitizer runs, and future Lua trace work.
- `make baseline` writes `.perf_baseline` so performance is scored against the
  current machine instead of CMU's Autolab hardware.
- ThreadSanitizer is used locally as the race detector when available.

## Academic Integrity Boundary

Do not copy an existing solution into `sfs-disk.c` if your goal is self-study.
The offline grader is meant to help you test your own implementation, not to
turn a submitted or third-party solution into a polished answer.

The starter functions in `sfs-disk.c` intentionally remain incomplete:

- `sfs_getpos`
- `sfs_seek`
- `sfs_rename`

Treat those as the warm-up portion of the lab.
