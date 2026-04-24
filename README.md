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
make report-json
make trace-check
make trace-smoke
make trace-run
make trace-json
make docker-check
make docker-report-json
make docker-trace-smoke
make docker-trace-run
make docker-trace-json
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

`make report-json` combines the local autograder JSON with the separate
official-style Lua trace coverage. The local autograder remains the graded
22-point score; trace coverage is diagnostic.

`make trace-smoke` builds the local Lua runner and executes the manifest's
starter-safe Lua traces. `make trace-run` executes the full Lua-style trace catalog and is
expected to fail on the unmodified skeleton. Both need `pkg-config` and Lua
development headers. `make trace-json` prints the same official-style trace
coverage as JSON; it is diagnostic and does not change the local 22-point
autograder score.

On Debian or Ubuntu, install those trace-runner dependencies with:

```bash
sudo apt-get install pkg-config lua5.4 liblua5.4-dev
```

The unmodified skeleton is expected to fail the tests for `sfs_getpos`,
`sfs_seek`, and `sfs_rename`. Those failures are the starting point of the lab,
not a packaging bug.

Docker works too:

```bash
docker build -t sfslab-offline .
docker run --rm -it -v "$PWD:/work" -w /work/sfslab sfslab-offline
```

The root Makefile wraps the same container flow:

```bash
make docker-check
make docker-report-json
make docker-trace-smoke
make docker-trace-run
make docker-trace-json
```

## What Is Local-Only

CMU's course infrastructure uses Autolab and internal race/concurrency tooling.
This offline version uses a C autograder, ThreadSanitizer when available, and a
machine-local performance baseline. The `sfslab/traces/` files are executable
Lua-style fixtures for the local runner. See
`sfslab/local/OFFLINE_SELF_STUDY.md` for the boundary between handout code and
local replacement tools. See `sfslab/local/SCORING.md` for how to read the
local score.

## Disclaimer

Original SFS lab materials belong to Carnegie Mellon University and the
15-213 / 15-513 course staff. This repository is an independent self-study
port with no CMU affiliation. Do not publish completed `sfs-disk.c` solutions.
