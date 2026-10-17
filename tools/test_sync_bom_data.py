"""Regression tests for sync_bom_data.py.

Run from the repository root: python3 -m unittest discover -s tools
"""

import contextlib
import io
import tempfile
import unittest
from pathlib import Path

import sync_bom_data as sync


class SlugifyTest(unittest.TestCase):
    def test_lowercases_and_hyphenates(self):
        self.assertEqual(sync.slugify("Vibration Sensor"), "vibration-sensor")

    def test_collapses_repeats_and_strips_edges(self):
        self.assertEqual(sync.slugify("  --Temp__&  Humidity!! "), "temp-humidity")

    def test_keeps_digits(self):
        self.assertEqual(sync.slugify("ADXL-345 v2"), "adxl-345-v2")


class SyncTestCase(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        root = Path(tmp.name)
        self.hardware = root / "hardware"
        self.data = root / "website" / "public" / "bom"

    def write(self, rel: str, text: str) -> None:
        path = self.hardware / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)


class CopyFileTest(SyncTestCase):
    def test_missing_source_is_skipped(self):
        changed, msg = sync.copy_file(
            self.hardware / "nope.csv", self.data / "out.csv", dry_run=False, verbose=False
        )
        self.assertFalse(changed)
        self.assertTrue(msg.startswith("SKIP"))
        self.assertFalse(self.data.exists())

    def test_copies_and_creates_directories(self):
        self.write("edge/bom.csv", "a,b\n")
        changed, msg = sync.copy_file(
            self.hardware / "edge" / "bom.csv", self.data / "bom-main.csv", dry_run=False, verbose=False
        )
        self.assertTrue(changed)
        self.assertTrue(msg.startswith("COPY"))
        self.assertEqual((self.data / "bom-main.csv").read_text(), "a,b\n")

    def test_overwrite_is_reported(self):
        self.write("edge/bom.csv", "new\n")
        self.data.mkdir(parents=True)
        (self.data / "bom-main.csv").write_text("old\n")
        changed, msg = sync.copy_file(
            self.hardware / "edge" / "bom.csv", self.data / "bom-main.csv", dry_run=False, verbose=False
        )
        self.assertTrue(changed)
        self.assertTrue(msg.startswith("OVERWRITE"))
        self.assertEqual((self.data / "bom-main.csv").read_text(), "new\n")

    def test_dry_run_writes_nothing(self):
        self.write("edge/bom.csv", "a\n")
        changed, msg = sync.copy_file(
            self.hardware / "edge" / "bom.csv", self.data / "bom-main.csv", dry_run=True, verbose=False
        )
        self.assertFalse(changed)
        self.assertTrue(msg.startswith("DRY-RUN"))
        self.assertFalse(self.data.exists())


class SyncSensorsTest(SyncTestCase):
    def test_missing_sensors_dir(self):
        messages = sync.sync_sensors(self.hardware, self.data, dry_run=False, verbose=False)
        self.assertEqual(len(messages), 1)
        self.assertTrue(messages[0].startswith("SKIP"))

    def test_each_sensor_gets_slugged_file(self):
        self.write("sensors/Vibration Sensor/bom.csv", "v\n")
        self.write("sensors/temp/bom.csv", "t\n")
        sync.sync_sensors(self.hardware, self.data, dry_run=False, verbose=False)
        self.assertEqual((self.data / "bom-vibration-sensor.csv").read_text(), "v\n")
        self.assertEqual((self.data / "bom-temp.csv").read_text(), "t\n")

    def test_sensor_without_bom_is_skipped_and_files_ignored(self):
        (self.hardware / "sensors" / "empty").mkdir(parents=True)
        (self.hardware / "sensors" / "README.md").write_text("not a dir\n")
        messages = sync.sync_sensors(self.hardware, self.data, dry_run=False, verbose=False)
        self.assertEqual(len(messages), 1)
        self.assertTrue(messages[0].startswith("SKIP"))


class MainTest(SyncTestCase):
    def run_main(self, *extra):
        argv = ["--hardware-dir", str(self.hardware), "--data-dir", str(self.data), *extra]
        with contextlib.redirect_stdout(io.StringIO()) as out:
            code = sync.main(argv)
        return code, out.getvalue()

    def populate(self):
        self.write("edge/bom.csv", "main\n")
        self.write("cables/bom.csv", "cables\n")
        self.write("sensors/pir/bom.csv", "pir\n")

    def test_all_copies_everything(self):
        self.populate()
        code, _ = self.run_main()
        self.assertEqual(code, 0)
        self.assertEqual(
            sorted(p.name for p in self.data.iterdir()),
            ["bom-cables.csv", "bom-main.csv", "bom-pir.csv"],
        )

    def test_only_cables(self):
        self.populate()
        self.run_main("--only", "cables")
        self.assertEqual([p.name for p in self.data.iterdir()], ["bom-cables.csv"])

    def test_only_sensors(self):
        self.populate()
        self.run_main("--only", "sensors")
        self.assertEqual([p.name for p in self.data.iterdir()], ["bom-pir.csv"])

    def test_dry_run_modifies_nothing(self):
        self.populate()
        code, out = self.run_main("--dry-run")
        self.assertEqual(code, 0)
        self.assertIn("Dry-run complete", out)
        self.assertFalse(self.data.exists())


if __name__ == "__main__":
    unittest.main()
