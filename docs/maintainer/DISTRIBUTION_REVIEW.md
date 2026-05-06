# Distribution Review Checklist

This repository-only checklist is for maintainers before publishing or sharing
the handout tarball. It is not legal advice.

## Project Boundary

- Confirm `README.md` and `NOTICE.md` say this is an independent self-study
  port with no CMU affiliation.
- Confirm the release does not imply endorsement by Carnegie Mellon University,
  15-213 / 15-513, or course staff.
- Confirm public-facing wording describes local packaging, diagnostics, and
  self-study support rather than an official course release.

## Contents

- Confirm `sfslab-handout.tar` contains only the `sfslab/` handout directory.
- Confirm the tarball does not include `.git/`, `.github/`, `docs/`, previous
  tarballs, local build products, or private notes.
- Confirm no completed `sfs-disk.c` solution or answer key is included.
- Confirm `sfs_getpos`, `sfs_seek`, and `sfs_rename` remain starter stubs in
  the public handout.

## Source Attribution

- Confirm `NOTICE.md` remains present in the repository.
- Confirm README disclaimer text remains present.
- Confirm any newly added material clearly belongs to the local self-study
  port: scripts, diagnostics, packaging notes, release checks, or local docs.

## Verification

- Run the maintainer release checks from `docs/maintainer/RELEASE.md`, including
  `make dist-verify`, `make dist-repro-check`, the compare helper unit test, the
  committed-tarball diff check, and the clean-worktree gate.
- Compare the fresh starter report's graded/core fields with the stored
  example:
  ```bash
  make report-json > /tmp/sfslab-report.json
  python3 -m json.tool /tmp/sfslab-report.json >/dev/null
  python3 docs/examples/compare-starter-report.py docs/examples/starter-report.json /tmp/sfslab-report.json
  python3 docs/examples/test_compare_starter_report.py
  git diff --exit-code -- sfslab-handout.tar
  test -z "$(git status --porcelain=v1 --untracked-files=all)"
  ```
- Do not require diagnostic sections to match exactly. Lua trace availability
  changes depending on whether local Lua development packages are installed.

## Release Decision

Do not publish if the review finds unclear attribution, bundled solutions,
private files, or repository-only maintenance material inside the handout
tarball.
