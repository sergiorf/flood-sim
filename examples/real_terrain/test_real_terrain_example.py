#!/usr/bin/env python3

from __future__ import annotations

import csv
import subprocess
import sys
from pathlib import Path

from fixtures import REAL_TERRAIN_FIXTURES, RealTerrainFixture


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


def run_example(binary_path: Path, terrain_path: Path, output_csv_path: Path, *args: str) -> subprocess.CompletedProcess[str]:
    output_csv_path.parent.mkdir(parents=True, exist_ok=True)
    return subprocess.run(
        [str(binary_path), str(terrain_path), str(output_csv_path), *args],
        check=True,
        capture_output=True,
        text=True,
    )


def assert_fixture_run(
    binary_path: Path,
    output_directory: Path,
    fixture: RealTerrainFixture,
) -> tuple[subprocess.CompletedProcess[str], dict[str, str], list[dict[str, str]]]:
    output_csv_path = output_directory / f"{fixture.name}.csv"
    completed = run_example(binary_path, fixture.terrain_path, output_csv_path, *fixture.scenario_args)

    assert f'loaded_dem="{fixture.terrain_path}"' in completed.stdout
    assert "rainfall_intensity_m_per_hour=0.012000" in completed.stdout
    assert "time_step_seconds=300.000" in completed.stdout
    assert "steps=12 " in completed.stdout
    assert "wrote_csv=" in completed.stdout
    assert output_csv_path.exists()

    for fragment in fixture.expected_stdout_fragments:
        assert fragment in completed.stdout

    metadata, rows = parse_export(output_csv_path)
    assert metadata == fixture.expected_metadata
    assert len(rows) == fixture.expected_row_count
    assert any(float(row["water_depth_m"]) > 0.0 for row in rows)
    assert expected_summary_line(rows) in completed.stdout
    assert f"deepest_row={fixture.expected_deepest_row}" in completed.stdout
    assert f"deepest_col={fixture.expected_deepest_col}" in completed.stdout

    return completed, metadata, rows


def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit("Usage: test_real_terrain_example.py <binary> <output_dir>")

    binary_path = Path(sys.argv[1])
    output_directory = Path(sys.argv[2])
    output_directory.mkdir(parents=True, exist_ok=True)

    for fixture in REAL_TERRAIN_FIXTURES:
        completed, metadata, rows = assert_fixture_run(binary_path, output_directory, fixture)
        assert expected_summary_line(rows) in completed.stdout
        assert metadata["scenario_name"] == "baseline"

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
