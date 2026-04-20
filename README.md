# cmu-sfslab-local

A self-contained, local-runnable version of CMU 15-213 / 15-513's
**Shark File System (SFS) Lab**, adapted for self-study on Ubuntu 18.04.

> Original lab (c) Carnegie Mellon University. This is a personal study
> port with no CMU affiliation — see the disclaimer at the bottom of this
> file and in `sfslab-local-handout/SFS_Lab_Writeup.md`.

---

## Repository Layout

```
cmu-sfslab-local/
├── README.md                        # this file
├── sfslab-local-handout/            # the handout, unpacked
│   ├── SFS_Lab_Writeup.md           #   full lab writeup (what to do)
│   ├── Makefile
│   ├── sfs-api.h                    #   API spec — read, do not modify
│   ├── sfs-disk.h                   #   on-disk structures
│   ├── sfs-disk.c                   #   *** the file you modify ***
│   ├── sfs-support.c                #   mmap / block I/O — do not modify
│   ├── sfs-baseline-ref.c           #   reference impl for perf calibration
│   ├── sfs-fsck.c                   #   disk consistency checker
│   └── test-sfs.c                   #   local autograder (all categories)
└── sfslab-local-handout.tar.gz      # the same handout, packaged
```

Use either the directory (for in-place work) or the tarball (for a clean
copy to share with someone else). They contain byte-identical sources.

---

## Background

CMU's original SFS lab depends on two course-internal pieces that a
non-CMU learner cannot use:

- **ConTech** — a proprietary race-detection tool built on LLVM
- **Autolab** — the staff-operated grading server with hardware-tuned
  performance thresholds

This repository replaces both with local-only equivalents:

| CMU original             | This repo                                              |
|--------------------------|--------------------------------------------------------|
| ConTech race detection   | ThreadSanitizer (`-fsanitize=thread`) build, auto-run  |
| Autolab perf thresholds  | Machine-local calibration via `make baseline` + ratios |
| Staff-maintained grader  | `test-sfs`, a self-contained C autograder              |

The writeup inside the handout (`SFS_Lab_Writeup.md`) explains the
filesystem design, the three functions you must implement (`sfs_getpos`,
`sfs_seek`, `sfs_rename`), and the suggested concurrency progression
(coarse lock → reader-writer → per-file).

---

## Quick Start

On Ubuntu 18.04 (or any glibc 2.27+ Linux with GCC 7.5+):

```bash
tar xzf sfslab-local-handout.tar.gz
cd sfslab-local-handout

make                  # builds sfs-fsck, test-sfs, test-sfs-baseline
make baseline         # one-time per machine — writes .perf_baseline
./test-sfs            # run the autograder
```

Expected first-run output (on the unmodified skeleton):

- Category A: 2/5 (format/mount and open/close work; getpos/seek/rename
  are the functions you implement)
- Category B: 1/3
- Category C: 3/3 (single-threaded trace skeletons)
- Perf: skipped (gated on 11/11 correctness)

As you implement functions and add locking, categories fill in and perf
starts producing a ratio against the reference implementation.

Full details, including how to interpret the scoreboard and how the
race detector is wired in, live in `sfslab-local-handout/SFS_Lab_Writeup.md`.

---

## What This Port Fixes vs. a Naive Local Grader

Two problems that a simple "copy the traces and run them" approach would
have, and how this repo handles them:

### 1. Student deadlocks must not hang the grader

Category C traces spawn worker threads and call `pthread_join`. A buggy
student lock strategy can deadlock — with a blocking join, the grader
then sits there forever and the student has to `Ctrl+C` to recover, losing
their partial score.

**Fix:** each Category C trace and the perf benchmark run in a **forked
child** with a wall-clock budget (30s / 60s). The parent polls
`waitpid(WNOHANG)` on a 100 ms cadence. On timeout, it `SIGKILL`s the
child, cleans up disk images, and scores the trace as 0. The grader is
guaranteed to finish in bounded time.

### 2. Performance thresholds must not be machine-dependent

CMU's autolab is calibrated on course-provided hardware, so they can
hardcode thresholds like "10000 ops/sec → 9/10". On your laptop the same
student code might score anywhere from 3/10 to 10/10 with zero change in
quality, purely because of CPU differences.

**Fix:** ship a **reference implementation** (`sfs-baseline-ref.c` — a
byte-identical copy of the handout `sfs-disk.c` wrapped with one global
`pthread_mutex`) and build it into `test-sfs-baseline`. `make baseline`
runs that binary's perf workload once, caches the ops/sec to
`.perf_baseline`, and the student grader scores by the ratio
`student_ops / baseline_ops`. A student who matches the reference scores
~1.0×; real reader-writer or per-file locking pushes well above 1.0×.

If `.perf_baseline` is missing, the grader falls back to legacy absolute
thresholds with a warning. If it's older than 30 days, the grader prints
a staleness notice suggesting re-calibration.

### Smaller robustness fixes

- `.perf_baseline` is resolved via `/proc/self/exe`, so `test-sfs` finds
  it regardless of the current working directory.
- Fork children call `fflush(stdout); fflush(stderr);` before `_exit`, so
  diagnostic lines survive output redirection (`./test-sfs > log.txt`).
- The baseline loader rejects NaN/Inf, not just non-positive values.

---

## Contributing / Reporting Issues

This is a personal study project, not a maintained product. If you spot
a bug or want to compare notes on the lab itself, open an issue — PRs
with bug fixes are welcome, but please don't post your `sfs-disk.c`
solution publicly (keep the lab self-contained for the next learner).

---

## Disclaimer

All original lab materials — including the SFS filesystem design, the
codebase skeleton, and the lab narrative — are the intellectual property
of **Carnegie Mellon University** and the 15-213 / 15-513 course staff.
This repository is an independent self-study port created by a non-CMU
learner and has **no affiliation with CMU**. If you are course staff and
this repository infringes on any rights, please open an issue and it
will be taken down immediately.

The port-specific additions in this repo (the local autograder, the
baseline calibration scheme, the deadlock-safe trace runner) are written
from scratch and released for educational use.
