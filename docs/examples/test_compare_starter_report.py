#!/usr/bin/env python3
import json
import subprocess
import sys
import os
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "docs" / "examples" / "compare-starter-report.py"
SAMPLE = ROOT / "docs" / "examples" / "starter-report.json"


def load_sample():
    with SAMPLE.open(encoding="utf-8") as handle:
        return json.load(handle)


class CompareStarterReportTests(unittest.TestCase):
    def run_compare(self, expected, actual):
        tmp_parent = os.environ.get("COMPARE_STARTER_REPORT_TMPDIR")
        tmp_options = {"dir": tmp_parent} if tmp_parent else {}
        with tempfile.TemporaryDirectory(
            prefix="compare-starter-report.", **tmp_options
        ) as tmp:
            tmp_path = Path(tmp)
            expected_path = tmp_path / "expected.json"
            actual_path = tmp_path / "actual.json"
            expected_path.write_text(json.dumps(expected), encoding="utf-8")
            actual_path.write_text(json.dumps(actual), encoding="utf-8")

            return subprocess.run(
                [sys.executable, str(SCRIPT), str(expected_path), str(actual_path)],
                check=False,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

    def test_ignores_environment_dependent_diagnostics(self):
        expected = load_sample()
        actual = load_sample()
        actual["official_style_traces"] = {
            "available": True,
            "exit_status": 0,
            "result": {"passed": True},
        }
        actual["stress_diagnostics"]["exit_status"] = 0
        actual["combined"]["diagnostics_passed"] = True

        result = self.run_compare(expected, actual)

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("starter report core fields match", result.stdout)

    def test_rejects_core_score_mismatch(self):
        expected = load_sample()
        actual = load_sample()
        actual["local_autograder"]["result"]["categories"]["A"]["score"] = 3

        result = self.run_compare(expected, actual)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("local_autograder.result.categories.A.score", result.stderr)


if __name__ == "__main__":
    unittest.main()
