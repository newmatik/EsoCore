"""Regression tests for step_linter.py.

Run from the repository root: python3 -m unittest discover -s tools
"""

import tempfile
import unittest
from pathlib import Path

from step_linter import StepLinter

AP242_FILE = """ISO-10303-21;
HEADER;
FILE_DESCRIPTION(('KiCad model'),'2;1');
FILE_NAME('old_name.step','2023-05-17T10:11:12',('Jane Doe'),('Acme'),'Preprocessor 1.0','KiCad','');
FILE_SCHEMA(('AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF { 1 0 10303 442 1 1 4 }'));
ENDSEC;
DATA;
#1=PRODUCT('part,revision','Part,Name','',(#2));
#2=PRODUCT_CONTEXT('',#3,'mechanical');
ENDSEC;
END-ISO-10303-21;
"""


class StepLinterTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.linter = StepLinter(target_path=self.tmp.name)

    def test_collapses_whole_run_of_duplicate_iso_lines(self):
        content = "ISO-10303-21;\nISO-10303-21;\nISO-10303-21;\nHEADER;"
        fixed, changed = self.linter.fix_duplicate_iso_lines(content)
        self.assertTrue(changed)
        self.assertEqual(fixed, "ISO-10303-21;\nHEADER;")

    def test_single_iso_line_is_unchanged(self):
        content = "ISO-10303-21;\nHEADER;"
        self.assertEqual(self.linter.fix_duplicate_iso_lines(content), (content, False))

    def test_quoted_strings_are_not_respaced(self):
        line = "#1=PRODUCT('part,revision','it''s,Name',#2,$);"
        self.assertEqual(
            self.linter.normalize_formatting(line),
            "#1 = PRODUCT('part,revision','it''s,Name', #2, $) ;",
        )

    def test_header_keeps_schema_timestamp_and_authors(self):
        fixed, changed = self.linter.fix_header(AP242_FILE, "new_name.step")
        self.assertTrue(changed)
        self.assertIn(
            "FILE_NAME('new_name.step','2023-05-17T10:11:12',('Jane Doe'),('Acme'),"
            "'Preprocessor 1.0','KiCad','');",
            fixed,
        )
        self.assertIn("FILE_SCHEMA(('AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF", fixed)
        self.assertIn("FILE_DESCRIPTION(('KiCad model'),'2;1');", fixed)
        self.assertNotIn("AUTOMOTIVE_DESIGN", fixed)
        # Only the FILE_NAME name changed.
        self.assertEqual(fixed.replace("new_name.step", "old_name.step"), AP242_FILE)

    def test_header_with_matching_name_is_unchanged(self):
        self.assertEqual(self.linter.fix_header(AP242_FILE, "old_name.step"), (AP242_FILE, False))

    def test_filename_with_quote_is_escaped(self):
        fixed, _ = self.linter.fix_header(AP242_FILE, "o'ring.step")
        self.assertIn("FILE_NAME('o''ring.step',", fixed)

    def test_process_file_end_to_end(self):
        path = Path(self.tmp.name) / "part.step"
        path.write_text("ISO-10303-21;\n" + AP242_FILE, encoding="utf-8")
        self.linter.process_file(str(path))
        result = path.read_text(encoding="utf-8")
        self.assertEqual(self.linter.stats["errors"], 0)
        self.assertTrue(result.startswith("ISO-10303-21;\nHEADER;"))
        self.assertIn("FILE_NAME('part.step','2023-05-17T10:11:12'", result)
        self.assertIn("AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF", result)
        self.assertIn("#1 = PRODUCT('part,revision','Part,Name','',(#2)) ;", result)


if __name__ == "__main__":
    unittest.main()
