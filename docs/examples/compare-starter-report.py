#!/usr/bin/env python3
"""Compare stable starter report fields and ignore environment diagnostics."""

import json
import sys
from pathlib import Path
from typing import Any


CORE_FIELDS = (
    "local_autograder.result.total.score",
    "local_autograder.result.total.max",
    "local_autograder.result.categories.A.score",
    "local_autograder.result.categories.A.max",
    "local_autograder.result.categories.B.score",
    "local_autograder.result.categories.B.max",
    "local_autograder.result.categories.C.score",
    "local_autograder.result.categories.C.max",
    "combined.graded_passed",
)


def load_json(path: Path) -> Any:
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def get_field(document: Any, dotted_path: str) -> Any:
    value = document
    for part in dotted_path.split("."):
        if not isinstance(value, dict) or part not in value:
            raise KeyError(dotted_path)
        value = value[part]
    return value


def compare(expected: Any, actual: Any) -> list[str]:
    mismatches = []
    for field in CORE_FIELDS:
        try:
            expected_value = get_field(expected, field)
            actual_value = get_field(actual, field)
        except KeyError:
            mismatches.append(f"{field}: missing")
            continue
        if expected_value != actual_value:
            mismatches.append(
                f"{field}: expected {expected_value!r}, got {actual_value!r}"
            )
    return mismatches


def main(argv: list[str]) -> int:
    if len(argv) != 3:
        print(
            "usage: compare-starter-report.py EXPECTED_JSON ACTUAL_JSON",
            file=sys.stderr,
        )
        return 2

    expected_path = Path(argv[1])
    actual_path = Path(argv[2])
    try:
        expected = load_json(expected_path)
        actual = load_json(actual_path)
    except (OSError, json.JSONDecodeError) as exc:
        print(f"compare-starter-report: {exc}", file=sys.stderr)
        return 2

    mismatches = compare(expected, actual)
    if mismatches:
        print("starter report core fields differ:", file=sys.stderr)
        for mismatch in mismatches:
            print(f"  {mismatch}", file=sys.stderr)
        return 1

    print("starter report core fields match")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
