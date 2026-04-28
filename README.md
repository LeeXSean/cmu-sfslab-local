# cmu-sfslab-local

Offline self-study packaging for CMU 15-213 / 15-513's Shark File System
(SFS) lab.

This is not an official CMU release. The goal is to keep the student-facing
handout close to the course shape while replacing CMU-only infrastructure with
local tools.

## Start Here

Run this inside Linux, WSL, or a Linux container:

```bash
tar xf sfslab-handout.tar
cd sfslab
make check
```

Then read `GETTING_STARTED.md` inside `sfslab/`. It gives the recommended
self-study order without adding extra grading requirements.

## Common Commands

From the repository root:

```bash
make doctor         # explain local toolchain readiness and fallbacks
make lint           # shell syntax plus optional ShellCheck/tool checks
make lint-strict    # require ShellCheck and clang-format availability
make check          # development health check
make json           # local 22-point score as JSON
make report-json    # score plus diagnostics; exits 0 if reporting worked
make report-json-strict  # same report, but fails when the graded score fails
make dist-check     # strict package cleanliness check
make dist-verify    # rebuild and unpack-check sfslab-handout.tar
make dist-repro-check  # build sfslab-handout.tar twice and compare hashes
make dist           # rebuild sfslab-handout.tar
```

Docker wrappers are also available:

```bash
make docker-check
make docker-dist-check
make docker-report-json
make docker-trace-smoke
```

On Ubuntu or WSL, the fuller local toolchain is:

```bash
sudo apt-get install build-essential make shellcheck clang-format pkg-config lua5.4 liblua5.4-dev
```

`make lint-strict` checks shell scripts with ShellCheck and verifies
clang-format is installed. To opt specific C files into format enforcement, set
`SFS_LINT_C_FORMAT_FILES`, for example:

```bash
SFS_LINT_C_FORMAT_FILES="local/test-report.c local/test-report.h" make lint-strict
```

## Project Layout

```text
cmu-sfslab-local/
  sfslab/              # handout-style working directory
  docs/RELEASE.md      # release verification checklist
  docs/MAINTAINERS.md  # repo-only maintenance notes
  docs/report-schema.md # report-json field reference
  sfslab-handout.tar   # packaged copy of sfslab/
```

Inside `sfslab/`:

- `GETTING_STARTED.md` explains the recommended workflow.
- `SFS_Lab_Writeup.md` is the lab narrative.
- `SCORING.md` explains the local score.
- `local/OFFLINE_SELF_STUDY.md` explains local-only differences.
- `traces/` contains executable Lua-style diagnostic traces.

## Boundary

The local 22-point score is frozen at A=5, B=4, C=3, Performance=10. Lua trace
coverage and stress diagnostics are reported separately and do not raise the
main score.

The starter intentionally leaves `sfs_getpos`, `sfs_seek`, and `sfs_rename`
incomplete. Those are part of the self-study work, not a packaging bug.
On a fresh starter, `make report-json` should produce valid JSON while reporting
that the graded score did not pass.

## Disclaimer

Original SFS lab materials belong to Carnegie Mellon University and the
15-213 / 15-513 course staff. This repository is an independent self-study
port with no CMU affiliation. Do not publish completed `sfs-disk.c` solutions.
See `NOTICE.md` for the release boundary.
