#!/usr/bin/env python3

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def parse_export(path: str | Path) -> dict:
    """Parse the documented FloodSim CSV export contract.

    The parser is intentionally strict so downstream examples fail fast if the
    export format changes unexpectedly.
    """
    export_path = Path(path)
    metadata: dict[str, str] = {}
    cells: list[dict[str, float | int]] = []

    with export_path.open("r", encoding="utf-8", newline="") as handle:
        header_seen = False

        for raw_line in handle:
            line = raw_line.strip()
            if not line:
                continue

            if line.startswith("# "):
                key, value = line[2:].split(",", 1)
                metadata[key] = value
                continue

            if not header_seen:
                if line != "row,col,elevation_m,water_depth_m,surface_height_m":
                    raise ValueError("Unexpected CSV header")
                header_seen = True
                continue

            row = next(csv.reader([line]))
            if len(row) != 5:
                raise ValueError("Unexpected CSV row width")

            cells.append({
                "row": int(row[0]),
                "col": int(row[1]),
                "elevation_m": float(row[2]),
                "water_depth_m": float(row[3]),
                "surface_height_m": float(row[4]),
            })

    if not header_seen:
        raise ValueError("Missing CSV header")

    required_metadata = {
        "floodsim_csv_version",
        "rows",
        "cols",
        "cell_size_m",
    }
    missing_metadata = required_metadata - metadata.keys()
    if missing_metadata:
        raise ValueError(f"Missing metadata keys: {sorted(missing_metadata)}")

    version = int(metadata["floodsim_csv_version"])
    if version != 1:
        raise ValueError(f"Unsupported floodsim_csv_version: {version}")

    rows = int(metadata["rows"])
    cols = int(metadata["cols"])
    expected_cells = rows * cols
    if len(cells) != expected_cells:
        raise ValueError(
            f"Expected {expected_cells} cell rows from metadata but found {len(cells)}"
        )

    parsed_metadata: dict[str, float | int | str] = {
        "floodsim_csv_version": version,
        "rows": rows,
        "cols": cols,
        "cell_size_m": float(metadata["cell_size_m"]),
    }
    if "origin_x_m" in metadata:
        parsed_metadata["origin_x_m"] = float(metadata["origin_x_m"])
    if "origin_y_m" in metadata:
        parsed_metadata["origin_y_m"] = float(metadata["origin_y_m"])
    if "crs_id" in metadata:
        parsed_metadata["crs_id"] = metadata["crs_id"]
    if "scenario_name" in metadata:
        parsed_metadata["scenario_name"] = metadata["scenario_name"]
    if "rainfall_intensity_m_per_hour" in metadata:
        parsed_metadata["rainfall_intensity_m_per_hour"] = float(
            metadata["rainfall_intensity_m_per_hour"]
        )
    if "time_step_seconds" in metadata:
        parsed_metadata["time_step_seconds"] = float(metadata["time_step_seconds"])
    if "total_duration_seconds" in metadata:
        parsed_metadata["total_duration_seconds"] = float(
            metadata["total_duration_seconds"]
        )

    return {
        "metadata": parsed_metadata,
        "cells": cells,
    }
    


def summarize_export(parsed_export: dict) -> dict[str, float | int]:
    metadata = parsed_export["metadata"]
    cells = parsed_export["cells"]
    total_water_depth_m = sum(cell["water_depth_m"] for cell in cells)
    max_water_depth_m = max((cell["water_depth_m"] for cell in cells), default=0.0)
    wet_cells = sum(1 for cell in cells if cell["water_depth_m"] > 0.0)

    summary: dict[str, float | int | str] = {
        "floodsim_csv_version": metadata["floodsim_csv_version"],
        "rows": metadata["rows"],
        "cols": metadata["cols"],
        "cell_size_m": metadata["cell_size_m"],
        "cells": len(cells),
        "wet_cells": wet_cells,
        "total_water_depth_m": total_water_depth_m,
        "max_water_depth_m": max_water_depth_m,
    }
    for optional_key in (
        "origin_x_m",
        "origin_y_m",
        "crs_id",
        "scenario_name",
        "rainfall_intensity_m_per_hour",
        "time_step_seconds",
        "total_duration_seconds",
    ):
        if optional_key in metadata:
            summary[optional_key] = metadata[optional_key]
    return summary


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Read a FloodSim Phase 1 CSV export and print a compact summary."
    )
    parser.add_argument("csv_path", help="Path to a FloodSim CSV export")
    args = parser.parse_args()

    summary = summarize_export(parse_export(args.csv_path))

    print(f"floodsim_csv_version={summary['floodsim_csv_version']}")
    print(f"rows={summary['rows']}")
    print(f"cols={summary['cols']}")
    print(f"cell_size_m={summary['cell_size_m']:.6f}")
    if "origin_x_m" in summary:
        print(f"origin_x_m={summary['origin_x_m']:.6f}")
    if "origin_y_m" in summary:
        print(f"origin_y_m={summary['origin_y_m']:.6f}")
    if "crs_id" in summary:
        print(f"crs_id={summary['crs_id']}")
    if "scenario_name" in summary:
        print(f"scenario_name={summary['scenario_name']}")
    if "rainfall_intensity_m_per_hour" in summary:
        print(
            "rainfall_intensity_m_per_hour="
            f"{summary['rainfall_intensity_m_per_hour']:.6f}"
        )
    if "time_step_seconds" in summary:
        print(f"time_step_seconds={summary['time_step_seconds']:.6f}")
    if "total_duration_seconds" in summary:
        print(f"total_duration_seconds={summary['total_duration_seconds']:.6f}")
    print(f"cells={summary['cells']}")
    print(f"wet_cells={summary['wet_cells']}")
    print(f"total_water_depth_m={summary['total_water_depth_m']:.6f}")
    print(f"max_water_depth_m={summary['max_water_depth_m']:.6f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
