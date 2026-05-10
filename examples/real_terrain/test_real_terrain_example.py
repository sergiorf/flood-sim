#!/usr/bin/env python3

from __future__ import annotations

import csv
import subprocess
import sys
from pathlib import Path


def parse_export(csv_path: Path) -> tuple[dict[str, str], list[dict[str, str]]]:
    metadata: dict[str, str] = {}
    rows: list[dict[str, str]] = []

    with csv_path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.reader(handle)
        header_seen = False

        for raw_row in reader:
            if not raw_row:
                continue

            if raw_row[0].startswith("# "):
                metadata[raw_row[0][2:]] = raw_row[1]
                continue

            if not header_seen:
                if raw_row != [
                    "row",
                    "col",
                    "elevation_m",
                    "water_depth_m",
                    "surface_height_m",
                ]:
                    raise AssertionError(f"Unexpected CSV header: {raw_row}")
                header_seen = True
                continue

            rows.append(
                {
                    "row": raw_row[0],
                    "col": raw_row[1],
                    "elevation_m": raw_row[2],
                    "water_depth_m": raw_row[3],
                    "surface_height_m": raw_row[4],
                }
            )

    return metadata, rows


def expected_summary_line(rows: list[dict[str, str]]) -> str:
    wet_rows = [row for row in rows if float(row["water_depth_m"]) > 0.0]
    wet_cells = len(wet_rows)
    if wet_rows:
        deepest = max(wet_rows, key=lambda row: float(row["water_depth_m"]))
        deepest_row = deepest["row"]
        deepest_col = deepest["col"]
        max_water_depth_m = float(deepest["water_depth_m"])
    else:
        deepest_row = "none"
        deepest_col = "none"
        max_water_depth_m = 0.0

    return (
        "summary_metrics "
        f"wet_cells={wet_cells} "
        f"max_water_depth_m={max_water_depth_m:.6f} "
        f"deepest_row={deepest_row} "
        f"deepest_col={deepest_col}"
    )


def main() -> int:
    if len(sys.argv) != 4:
        raise SystemExit(
            "Usage: test_real_terrain_example.py <binary> <input_dem.tif> <output.csv>"
        )

    binary_path = Path(sys.argv[1])
    input_dem_path = Path(sys.argv[2])
    output_csv_path = Path(sys.argv[3])
    output_csv_path.parent.mkdir(parents=True, exist_ok=True)

    completed = subprocess.run(
        [str(binary_path), str(input_dem_path), str(output_csv_path)],
        check=True,
        capture_output=True,
        text=True,
    )

    assert "loaded_dem=" in completed.stdout
    assert "scenario_name=baseline scenario_source=direct_cli_or_default" in completed.stdout
    assert (
        "ingestion_report source_rows=5 source_cols=5 loaded_rows=5 loaded_cols=5 "
        "clipped_cells=0 invalid_cells=1 nodata_metadata_present=true nan_cells=0 "
        "nodata_status=band_metadata_applied"
    ) in completed.stdout
    assert "rainfall_intensity_m_per_hour=0.012000" in completed.stdout
    assert "time_step_seconds=300.000" in completed.stdout
    assert "steps=12 " in completed.stdout
    assert "wrote_csv=" in completed.stdout
    assert output_csv_path.exists()

    metadata, rows = parse_export(output_csv_path)
    assert metadata == {
        "floodsim_csv_version": "1",
        "rows": "5",
        "cols": "5",
        "cell_size_m": "2.000000",
        "scenario_name": "baseline",
        "rainfall_intensity_m_per_hour": "0.012000",
        "time_step_seconds": "300.000000",
        "total_duration_seconds": "3600.000000",
        "origin_x_m": "154320.000000",
        "origin_y_m": "171205.000000",
        "crs_id": "EPSG:31370",
    }
    assert len(rows) == 25
    assert any(float(row["water_depth_m"]) > 0.0 for row in rows)
    assert any(float(row["elevation_m"]) < 0.0 for row in rows)
    assert expected_summary_line(rows) in completed.stdout

    custom_output_csv_path = output_csv_path.with_name("output_custom.csv")
    custom_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(custom_output_csv_path),
            "--rainfall-intensity-m-per-hour",
            "0.020",
            "--time-step-seconds",
            "600",
            "--steps",
            "4",
        ],
        check=True,
        capture_output=True,
        text=True,
    )

    assert "rainfall_intensity_m_per_hour=0.020000" in custom_completed.stdout
    assert "time_step_seconds=600.000" in custom_completed.stdout
    assert "steps=4 " in custom_completed.stdout
    assert custom_output_csv_path.exists()
    custom_metadata, custom_rows = parse_export(custom_output_csv_path)
    assert custom_metadata["scenario_name"] == "baseline"
    assert expected_summary_line(custom_rows) in custom_completed.stdout

    preset_output_csv_path = output_csv_path.with_name("output_preset.csv")
    preset_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(preset_output_csv_path),
            "--scenario",
            "intense_short",
        ],
        check=True,
        capture_output=True,
        text=True,
    )

    assert "scenario_name=intense_short scenario_source=preset" in preset_completed.stdout
    assert "rainfall_intensity_m_per_hour=0.030000" in preset_completed.stdout
    assert "time_step_seconds=300.000" in preset_completed.stdout
    assert "steps=6 " in preset_completed.stdout
    assert preset_output_csv_path.exists()
    preset_metadata, _ = parse_export(preset_output_csv_path)
    assert preset_metadata["scenario_name"] == "intense_short"
    assert preset_metadata["rainfall_intensity_m_per_hour"] == "0.030000"
    assert preset_metadata["time_step_seconds"] == "300.000000"
    assert preset_metadata["total_duration_seconds"] == "1800.000000"
    assert expected_summary_line(parse_export(preset_output_csv_path)[1]) in preset_completed.stdout

    override_output_csv_path = output_csv_path.with_name("output_preset_override.csv")
    override_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(override_output_csv_path),
            "--rainfall-intensity-m-per-hour",
            "0.018",
            "--scenario",
            "long_moderate",
            "--steps",
            "10",
        ],
        check=True,
        capture_output=True,
        text=True,
    )

    assert (
        "scenario_name=long_moderate scenario_source=preset_with_cli_overrides"
        in override_completed.stdout
    )
    assert "rainfall_intensity_m_per_hour=0.018000" in override_completed.stdout
    assert "time_step_seconds=300.000" in override_completed.stdout
    assert "steps=10 " in override_completed.stdout
    assert override_output_csv_path.exists()
    override_metadata, _ = parse_export(override_output_csv_path)
    assert override_metadata["scenario_name"] == "long_moderate"
    assert override_metadata["rainfall_intensity_m_per_hour"] == "0.018000"
    assert override_metadata["time_step_seconds"] == "300.000000"
    assert override_metadata["total_duration_seconds"] == "3000.000000"
    assert expected_summary_line(parse_export(override_output_csv_path)[1]) in override_completed.stdout

    clipped_output_csv_path = output_csv_path.with_name("output_clipped.csv")
    clipped_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(clipped_output_csv_path),
            "--window-row-offset",
            "1",
            "--window-col-offset",
            "1",
            "--window-rows",
            "3",
            "--window-cols",
            "2",
        ],
        check=True,
        capture_output=True,
        text=True,
    )

    assert "window_row_offset=1 window_col_offset=1 window_rows=3 window_cols=2" in clipped_completed.stdout
    assert (
        "ingestion_report source_rows=5 source_cols=5 loaded_rows=3 loaded_cols=2 "
        "clipped_cells=19 invalid_cells=0 nodata_metadata_present=true nan_cells=0 "
        "nodata_status=band_metadata_applied"
    ) in clipped_completed.stdout
    clipped_metadata, clipped_rows = parse_export(clipped_output_csv_path)
    assert clipped_metadata["rows"] == "3"
    assert clipped_metadata["cols"] == "2"
    assert clipped_metadata["scenario_name"] == "baseline"
    assert clipped_metadata["origin_x_m"] == "154322.000000"
    assert clipped_metadata["origin_y_m"] == "171203.000000"
    assert len(clipped_rows) == 6
    assert expected_summary_line(clipped_rows) in clipped_completed.stdout

    invalid_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(output_csv_path.with_name("output_invalid.csv")),
            "--steps",
            "0",
        ],
        check=False,
        capture_output=True,
        text=True,
    )

    assert invalid_completed.returncode != 0
    assert "Step count must be positive" in invalid_completed.stderr

    invalid_time_step_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(output_csv_path.with_name("output_invalid_time_step.csv")),
            "--time-step-seconds",
            "0",
        ],
        check=False,
        capture_output=True,
        text=True,
    )

    assert invalid_time_step_completed.returncode != 0
    assert "Time step must be positive" in invalid_time_step_completed.stderr

    invalid_scenario_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(output_csv_path.with_name("output_invalid_scenario.csv")),
            "--scenario",
            "not_a_real_preset",
        ],
        check=False,
        capture_output=True,
        text=True,
    )

    assert invalid_scenario_completed.returncode != 0
    assert "Unknown scenario preset: not_a_real_preset" in invalid_scenario_completed.stderr

    invalid_window_completed = subprocess.run(
        [
            str(binary_path),
            str(input_dem_path),
            str(output_csv_path.with_name("output_invalid_window.csv")),
            "--window-row-offset",
            "1",
            "--window-cols",
            "2",
        ],
        check=False,
        capture_output=True,
        text=True,
    )

    assert invalid_window_completed.returncode != 0
    assert "Terrain window requires both --window-rows and --window-cols" in invalid_window_completed.stderr

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
