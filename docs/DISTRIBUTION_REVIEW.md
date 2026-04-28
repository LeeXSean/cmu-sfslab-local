# Distribution Review Checklist

This checklist is for maintainers before publishing or sharing the handout
tarball. It is not legal advice.

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

- Run `make dist-verify`.
- Run `make dist-repro-check`.
- Compare the fresh starter report with `docs/examples/starter-report.json`.

## Release Decision

Do not publish if the review finds unclear attribution, bundled solutions,
private files, or repository-only maintenance material inside the handout
tarball.
