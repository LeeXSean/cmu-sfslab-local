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
  docs/MAINTAINERS.md     # repo-only maintenance notes
  sfslab/                  # handout-style working directory
    README
    GETTING_STARTED.md
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

Inside the handout, start with `GETTING_STARTED.md`; it gives the recommended
order without adding extra grading requirements.

Helpful targets inside `sfslab/`:

```bash
make test         # run the local autograder
make json         # print the local autograder score as JSON
make grade        # recalibrate baseline, then run the autograder
make smoke        # verify the starter traces run cleanly
make starter-safe # run starter-safe C checks and Lua traces
make trace-check  # syntax-check Lua-style traces when luac is installed
```

The repository root also forwards common targets:

```bash
make smoke
make starter-safe
make stress
make check
make json
make report-json
make trace-check
make trace-smoke
make trace-run
make trace-json
make docker-check
make docker-dist-check
make docker-starter-safe
make docker-stress
make docker-report-json
make docker-trace-smoke
make docker-trace-run
make docker-trace-json
make trace-list
make manifest-check
make handout-check
make starter-check
make dist-check
make dist
```

GitHub Actions runs the same smoke and trace syntax checks on pushes and pull
requests.

`make check` is the development health check and can be run after normal build
or grading targets. `make dist-check` is the stricter package check: it includes
`handout-check`, so it expects no generated build artifacts to be present.

`make dist` runs `make clean`, `make dist-check`, and another `make clean`,
then uses reproducible GNU tar flags when they are available.

`make starter-safe` is the starter health check: it runs the C smoke traces and
then runs the manifest's starter-safe Lua traces. From the repository root, Lua
trace targets fall back to Docker when local Lua development dependencies are
missing.

`make json` exits nonzero when correctness fails, just like `make test`; the
JSON is still written to stdout.

Correctness traces also run `sfs-fsck` after unmounting their disk images, so
metadata corruption can fail a trace even when the last read/write result looked
right. The failure detail includes the first captured checker diagnostic.

`make report-json` combines the local autograder JSON with separate
official-style Lua trace coverage and stress diagnostics. The local autograder
remains the graded 22-point score; Lua and stress sections are diagnostic.

`make stress` repeats the C concurrency traces and then runs extra diagnostic
stress checks that are intentionally outside the 22-point score. It is an
implementation stability tool, not a starter health check; the unmodified
skeleton may fail it because its concurrency path is intentionally not fixed
yet.

`make trace-smoke` builds the local Lua runner and executes the manifest's
starter-safe Lua traces. `make trace-run` executes the full Lua-style trace
catalog and is expected to fail on the unmodified skeleton. The runner provides
a pthread-backed `lanes.gen` compatibility layer, so Lua C traces exercise real
concurrent calls into the SFS API. `make trace-json` prints the same
official-style trace coverage as JSON; it is diagnostic and does not change the
local 22-point autograder score.

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
make docker-dist-check
make docker-starter-safe
make docker-stress
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
local score. Maintainer-only notes live in `docs/MAINTAINERS.md`, outside the
student handout.

## Disclaimer

Original SFS lab materials belong to Carnegie Mellon University and the
15-213 / 15-513 course staff. This repository is an independent self-study
port with no CMU affiliation. Do not publish completed `sfs-disk.c` solutions.
