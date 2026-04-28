# Release Checklist

Use this checklist before publishing or handing off `sfslab-handout.tar`.

```bash
make doctor
make lint-strict
make clean
make check
make trace-smoke
make report-json
make dist-verify
make dist-repro-check
```

`make dist-verify` rebuilds `sfslab-handout.tar`, extracts it into a temporary
directory, runs `make check` from the extracted handout, and validates that
`make report-json` emits parseable JSON.

`make dist-repro-check` rebuilds the tarball twice and compares SHA-256 hashes.
If it fails, the release artifact is carrying unstable metadata or generated
contents.

Expected starter status:

- `make lint-strict` passes when ShellCheck and clang-format are installed.
  C formatting is opt-in through `SFS_LINT_C_FORMAT_FILES` because the inherited
  handout C sources are not all clang-format clean.
- `make check` passes.
- `make trace-smoke` passes the starter-safe subset.
- `make report-json` exits 0 because report generation worked.
- The JSON score still reports the intentionally incomplete starter as not
  fully graded.
- `sfs_getpos`, `sfs_seek`, and `sfs_rename` remain `-ENOSYS` stubs.
