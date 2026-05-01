#!/usr/bin/env python3
"""
Placeholder preprocessing entrypoint for future terrain ingestion.

The MVP does not depend on GDAL yet. This script exists to define where
GeoTIFF or DEM preparation logic should live once ingestion work begins.
"""

from pathlib import Path


def main() -> None:
    repo_root = Path(__file__).resolve().parents[2]
    print("FloodSim preprocessing stub")
    print(f"Repository root: {repo_root}")
    print("Next step: add DEM/GeoTIFF ingestion, likely behind an optional GDAL dependency.")


if __name__ == "__main__":
    main()

