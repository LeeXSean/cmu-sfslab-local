# cmu-sfslab-local

Offline self-study packaging for CMU 15-213 / 15-513's Shark File System
(SFS) lab.

This repository is not an official CMU release. The goal is to keep the
student-facing handout directory close to the course handout shape while
replacing CMU-only infrastructure with local tools.

## Layout

```text
cmu-sfslab-local/
  README.md
  sfslab/                  # handout-style working directory
    README
    SFS_Lab_Writeup.md
    Makefile
    sfs-disk.c             # student file
    sfs-api.h
    sfs-disk.h
    sfs-support.c
    sfs-fsck.c
    traces/                # Lua-style trace catalog
    local/                 # offline-only replacement tooling
      test-sfs.c
      sfs-baseline-ref.c
      OFFLINE_SELF_STUDY.md
  sfslab-handout.tar       # packaged copy of sfslab/
```

## Quick Start

Run this inside Linux, WSL, or a Linux container:

```bash
tar xf sfslab-handout.tar
cd sfslab
make
make baseline
./test-sfs
```

Helpful targets inside `sfslab/`:

```bash
make test         # run the local autograder
make json         # print the local autograder score as JSON
make grade        # recalibrate baseline, then run the autograder
make smoke        # verify the starter traces run cleanly
make trace-check  # syntax-check Lua-style traces when luac is installed
```

The repository root also forwards common targets:

```bash
make smoke
make check
make json
make trace-check
make trace-list
make manifest-check
make handout-check
make starter-check
make dist
```

GitHub Actions runs the same smoke and trace syntax checks on pushes and pull
requests.

`make dist` runs the handout, starter, and manifest checks first, then uses
reproducible GNU tar flags when they are available.

`make json` exits nonzero when correctness fails, just like `make test`; the
JSON is still written to stdout.

The unmodified skeleton is expected to fail the tests for `sfs_getpos`,
`sfs_seek`, and `sfs_rename`. Those failures are the starting point of the lab,
not a packaging bug.

Docker works too:

```bash
docker build -t sfslab-offline .
docker run --rm -it -v "$PWD:/work" -w /work/sfslab sfslab-offline
```

## What Is Local-Only

CMU's course infrastructure uses Autolab and internal race/concurrency tooling.
This offline version uses a C autograder, ThreadSanitizer when available, and a
machine-local performance baseline. The `sfslab/traces/` files document the
Lua-style test shape but are not executed yet. See
`sfslab/local/OFFLINE_SELF_STUDY.md` for the boundary between handout code and
local replacement tools. See `sfslab/local/SCORING.md` for how to read the
local score.

## Disclaimer

Original SFS lab materials belong to Carnegie Mellon University and the
15-213 / 15-513 course staff. This repository is an independent self-study
port with no CMU affiliation. Do not publish completed `sfs-disk.c` solutions.
