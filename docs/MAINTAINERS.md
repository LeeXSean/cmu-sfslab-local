# Maintainer Notes

These notes are for maintaining the offline self-study port. They are not part
of the student solution.

## Student API Boundary

`sfs-api.h` is the lab contract. Keep it stable unless the whole handout,
autograder, trace runner, reference implementation, and writeup are changed
together.

The historical in-header TODO mentioned adding file-size operations such as
`fstat` and `ftruncate`. Do not make those required for the current local score.
They would add new allocation and truncation semantics that are outside the
existing lab surface.

If file-size operations are added later, keep them as optional local extensions:

- expose them through a separate target or report section;
- keep them out of the 22-point local score;
- add Lua runner bindings, C tests, reference behavior, and writeup text before
  publishing them.

The historical TODO also mentioned wrapping SFS file descriptors in a C newtype.
Keep the public API as `int fd`, because the existing handout, tests, and trace
style all use that shape. If fd confusion becomes a real maintenance issue,
prefer internal helper functions in the local test harness over changing the
student-facing function signatures.

## Inherited Support TODOs

`sfs-fsck.c` still contains small inherited FIXME comments around ASCII name
assumptions and block-device sizing. They are not part of the student task and
do not affect the local grading path. Treat them as support-tool maintenance,
not as assignment requirements.

## Core Next Work

- Add more correctness traces before changing score weights.
- Keep Lua trace coverage diagnostic until it matches the C autograder's
  concurrency signal.
- Use the trace manifest's starter metadata to keep smoke checks aligned with
  the Lua catalog.
- Keep schedule-sensitive concurrency churn in stress-only diagnostics, not in
  the main local score.
- Keep report formatting in `test-report.c` so `test-sfs.c` stays focused on
  trace behavior.
- Prefer local reporting and reproducibility improvements over expanding the
  public SFS API.
