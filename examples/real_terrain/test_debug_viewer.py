#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from debug_viewer import (
    convert_raster_to_viewer_csv,
    dataset_for_layer,
    discover_snapshot_series,
    finite_range,
    format_value,
    load_frames,
    parse_esri_ascii_grid,
    parse_floodsim_csv,
    series_range_for_layer,
    should_draw_value_overlay,
    text_color_for_hex,
)

TERRAIN_EXPORT_BINARY: Path | None = None


class DebugViewerParsingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.terrain_export_binary = TERRAIN_EXPORT_BINARY

    def test_parse_esri_ascii_grid_reads_fixture(self) -> None:
        fixture_path = Path(__file__).resolve().parent / "data" / "flat_pond.asc"
        frame = parse_esri_ascii_grid(fixture_path)

        self.assertEqual(frame.rows, 5)
        self.assertEqual(frame.cols, 5)
        self.assertEqual(frame.metadata["source_format"], "esri_ascii_grid")
        self.assertEqual(frame.metadata["origin_x_m"], "8000")
        self.assertEqual(frame.metadata["origin_y_m"], "2025.0")
        self.assertIsNone(frame.water_depth)
        self.assertEqual(frame.elevation[12], 98.0)

    def test_parse_floodsim_csv_reads_export_contract(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            csv_path = Path(temp_dir) / "export.csv"
            csv_path.write_text(
                "# floodsim_csv_version,1\n"
                "# rows,2\n"
                "# cols,2\n"
                "# cell_size_m,5.000000\n"
                "# scenario_name,baseline\n"
                "row,col,elevation_m,water_depth_m,surface_height_m\n"
                "0,0,100.000000,0.000000,100.000000\n"
                "0,1,99.000000,0.010000,99.010000\n"
                "1,0,98.000000,0.020000,98.020000\n"
                "1,1,97.000000,0.030000,97.030000\n",
                encoding="utf-8",
            )

            frame = parse_floodsim_csv(csv_path)

            self.assertEqual(frame.rows, 2)
            self.assertEqual(frame.cols, 2)
            self.assertEqual(frame.metadata["scenario_name"], "baseline")
            self.assertEqual(frame.water_depth, [0.0, 0.01, 0.02, 0.03])
            self.assertEqual(frame.surface_height, [100.0, 99.01, 98.02, 97.03])

    def test_discover_snapshot_series_sorts_snapshots_before_final(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            final_path = temp_path / "run.csv"
            step_four = temp_path / "run_step0004_t1200s.csv"
            step_eight = temp_path / "run_step0008_t2400s.csv"
            for path in (final_path, step_eight, step_four):
                path.write_text("", encoding="utf-8")

            discovered = discover_snapshot_series(final_path)

            self.assertEqual(discovered, [step_four, step_eight, final_path])

    def test_load_frames_rejects_missing_csv_series(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            missing_path = Path(temp_dir) / "missing.csv"

            with self.assertRaisesRegex(FileNotFoundError, "No FloodSim CSV frames found"):
                load_frames([str(missing_path)])

    def test_format_value_formats_finite_and_nodata(self) -> None:
        self.assertEqual(format_value(12.34567), "12.346")
        self.assertEqual(format_value(float("nan")), "nodata")

    def test_dataset_for_layer_selects_requested_series(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            csv_path = Path(temp_dir) / "export.csv"
            csv_path.write_text(
                "# floodsim_csv_version,1\n"
                "# rows,1\n"
                "# cols,2\n"
                "row,col,elevation_m,water_depth_m,surface_height_m\n"
                "0,0,10.000000,1.000000,11.000000\n"
                "0,1,12.000000,0.500000,12.500000\n",
                encoding="utf-8",
            )
            frame = parse_floodsim_csv(csv_path)

            dataset, palette = dataset_for_layer(frame, "water_depth")
            self.assertEqual(dataset, [1.0, 0.5])
            self.assertEqual(palette, "water")

    def test_finite_range_ignores_nodata(self) -> None:
        self.assertEqual(finite_range([float("nan"), 2.0, 5.0]), (2.0, 5.0))
        self.assertEqual(finite_range([float("nan")]), (0.0, 0.0))

    def test_series_range_for_layer_uses_all_frames(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            first_path = Path(temp_dir) / "first.csv"
            second_path = Path(temp_dir) / "second.csv"
            first_path.write_text(
                "# floodsim_csv_version,1\n"
                "# rows,1\n"
                "# cols,1\n"
                "row,col,elevation_m,water_depth_m,surface_height_m\n"
                "0,0,10.000000,0.100000,10.100000\n",
                encoding="utf-8",
            )
            second_path.write_text(
                "# floodsim_csv_version,1\n"
                "# rows,1\n"
                "# cols,1\n"
                "row,col,elevation_m,water_depth_m,surface_height_m\n"
                "0,0,12.000000,0.300000,12.300000\n",
                encoding="utf-8",
            )
            frames = [parse_floodsim_csv(first_path), parse_floodsim_csv(second_path)]

            self.assertEqual(series_range_for_layer(frames, "water_depth"), (0.1, 0.3))

    def test_should_draw_value_overlay_only_when_readable(self) -> None:
        self.assertTrue(should_draw_value_overlay(5, 5, 100.0, 100.0))
        self.assertFalse(should_draw_value_overlay(11, 11, 100.0, 100.0))
        self.assertFalse(should_draw_value_overlay(5, 5, 40.0, 100.0))
        self.assertFalse(should_draw_value_overlay(5, 5, 100.0, 20.0))

    def test_text_color_for_hex_prefers_contrast(self) -> None:
        self.assertEqual(text_color_for_hex("#f0f0f0"), "#111111")
        self.assertEqual(text_color_for_hex("#202020"), "#f2f2f2")

    def test_convert_raster_to_viewer_csv_uses_terrain_export_helper(self) -> None:
        if self.terrain_export_binary is None:
            self.skipTest("terrain export helper path not provided")

        fixture_path = Path(__file__).resolve().parent / "data" / "sample_dem.tif"
        csv_path = convert_raster_to_viewer_csv(fixture_path, self.terrain_export_binary)
        frame = parse_floodsim_csv(csv_path)

        self.assertEqual(frame.rows, 5)
        self.assertEqual(frame.cols, 5)
        self.assertEqual(frame.metadata["origin_x_m"], "154320.000000")
        self.assertEqual(frame.metadata["origin_y_m"], "171205.000000")
        self.assertEqual(frame.metadata["crs_id"], "EPSG:31370")
        self.assertEqual(frame.water_depth.count(0.0), 25)


if __name__ == "__main__":
    import sys

    if len(sys.argv) > 1:
        TERRAIN_EXPORT_BINARY = Path(sys.argv[1])
    sys.argv = [sys.argv[0]]
    unittest.main()
