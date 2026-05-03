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

    return {
        "metadata": {
            "floodsim_csv_version": version,
            "rows": rows,
            "cols": cols,
            "cell_size_m": float(metadata["cell_size_m"]),
        },
        "cells": cells,
    }


def summarize_export(parsed_export: dict) -> dict[str, float | int]:
    metadata = parsed_export["metadata"]
    cells = parsed_export["cells"]
    total_water_depth_m = sum(cell["water_depth_m"] for cell in cells)
    max_water_depth_m = max((cell["water_depth_m"] for cell in cells), default=0.0)
    wet_cells = sum(1 for cell in cells if cell["water_depth_m"] > 0.0)

    return {
        "floodsim_csv_version": metadata["floodsim_csv_version"],
        "rows": metadata["rows"],
        "cols": metadata["cols"],
        "cell_size_m": metadata["cell_size_m"],
        "cells": len(cells),
        "wet_cells": wet_cells,
        "total_water_depth_m": total_water_depth_m,
        "max_water_depth_m": max_water_depth_m,
    }


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
    print(f"cells={summary['cells']}")
    print(f"wet_cells={summary['wet_cells']}")
    print(f"total_water_depth_m={summary['total_water_depth_m']:.6f}")
    print(f"max_water_depth_m={summary['max_water_depth_m']:.6f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
