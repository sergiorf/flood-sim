#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import textwrap
import unittest
from pathlib import Path

import inspect_export


class InspectExportTests(unittest.TestCase):
    def test_parse_and_summarize_export(self) -> None:
        sample_export = textwrap.dedent(
            """\
            # floodsim_csv_version,1
            # rows,2
            # cols,2
            # cell_size_m,2.500000
            row,col,elevation_m,water_depth_m,surface_height_m
            0,0,1.000000,0.250000,1.250000
            0,1,1.500000,0.000000,1.500000
            1,0,2.000000,0.100000,2.100000
            1,1,2.500000,0.750000,3.250000
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir:
            export_path = Path(temp_dir) / "sample.csv"
            export_path.write_text(sample_export, encoding="utf-8")

            parsed = inspect_export.parse_export(export_path)
            summary = inspect_export.summarize_export(parsed)

        self.assertEqual(parsed["metadata"]["floodsim_csv_version"], 1)
        self.assertEqual(parsed["metadata"]["rows"], 2)
        self.assertEqual(parsed["metadata"]["cols"], 2)
        self.assertAlmostEqual(parsed["metadata"]["cell_size_m"], 2.5)
        self.assertEqual(len(parsed["cells"]), 4)

        self.assertEqual(summary["cells"], 4)
        self.assertEqual(summary["wet_cells"], 3)
        self.assertAlmostEqual(summary["total_water_depth_m"], 1.1)
        self.assertAlmostEqual(summary["max_water_depth_m"], 0.75)

    def test_parse_export_preserves_optional_georeferencing_metadata(self) -> None:
        sample_export = textwrap.dedent(
            """\
            # floodsim_csv_version,1
            # rows,1
            # cols,2
            # cell_size_m,2.000000
            # origin_x_m,154320.000000
            # origin_y_m,171205.000000
            # crs_id,EPSG:31370
            row,col,elevation_m,water_depth_m,surface_height_m
            0,0,1.000000,0.250000,1.250000
            0,1,1.500000,0.000000,1.500000
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir:
            export_path = Path(temp_dir) / "sample.csv"
            export_path.write_text(sample_export, encoding="utf-8")

            parsed = inspect_export.parse_export(export_path)
            summary = inspect_export.summarize_export(parsed)

        self.assertAlmostEqual(parsed["metadata"]["origin_x_m"], 154320.0)
        self.assertAlmostEqual(parsed["metadata"]["origin_y_m"], 171205.0)
        self.assertEqual(parsed["metadata"]["crs_id"], "EPSG:31370")
        self.assertAlmostEqual(summary["origin_x_m"], 154320.0)
        self.assertAlmostEqual(summary["origin_y_m"], 171205.0)
        self.assertEqual(summary["crs_id"], "EPSG:31370")

    def test_parse_export_rejects_unsupported_version(self) -> None:
        sample_export = textwrap.dedent(
            """\
            # floodsim_csv_version,2
            # rows,1
            # cols,1
            # cell_size_m,1.000000
            row,col,elevation_m,water_depth_m,surface_height_m
            0,0,1.000000,0.250000,1.250000
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir:
            export_path = Path(temp_dir) / "sample.csv"
            export_path.write_text(sample_export, encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "Unsupported floodsim_csv_version"):
                inspect_export.parse_export(export_path)

    def test_parse_export_rejects_wrong_cell_count(self) -> None:
        sample_export = textwrap.dedent(
            """\
            # floodsim_csv_version,1
            # rows,2
            # cols,2
            # cell_size_m,1.000000
            row,col,elevation_m,water_depth_m,surface_height_m
            0,0,1.000000,0.250000,1.250000
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir:
            export_path = Path(temp_dir) / "sample.csv"
            export_path.write_text(sample_export, encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "Expected 4 cell rows"):
                inspect_export.parse_export(export_path)


if __name__ == "__main__":
    unittest.main()
