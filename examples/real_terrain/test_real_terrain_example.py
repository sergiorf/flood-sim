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
    assert "wrote_csv=" in completed.stdout
    assert output_csv_path.exists()

    metadata, rows = parse_export(output_csv_path)
    assert metadata == {
        "floodsim_csv_version": "1",
        "rows": "5",
        "cols": "5",
        "cell_size_m": "2.000000",
    }
    assert len(rows) == 25
    assert any(float(row["water_depth_m"]) > 0.0 for row in rows)
    assert any(float(row["elevation_m"]) < 0.0 for row in rows)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
