#!/usr/bin/env python3
"""
Validate the FloodSim terrain library metadata and onboarding protocol shape.
"""

from __future__ import annotations

import csv
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
CATALOG_PATH = REPO_ROOT / "terrain_library" / "catalog.csv"

ALLOWED_INTENDED_USE = {"demo", "screening", "regression_fixture", "research"}
ALLOWED_STATUS = {"candidate", "reviewed", "active_demo", "archived"}
ALLOWED_TERRAIN_TYPE = {"DEM", "DTM", "DSM"}
REQUIRED_AREA_SECTIONS = (
    "## Status",
    "## Why this area",
    "## Layer inventory",
    "## Source candidates",
    "## Open questions",
    "## First acceptance target",
    "## Screening limitations",
)
EXPECTED_HEADER = [
    "area_id",
    "display_name",
    "city_or_region",
    "country",
    "intended_use",
    "status",
    "terrain_type",
    "source_name",
    "source_url",
    "license_name",
    "resolution",
    "crs",
    "local_files",
    "notes",
]


def main() -> int:
    errors: list[str] = []
    if not CATALOG_PATH.exists():
        errors.append(f"Missing catalog file: {CATALOG_PATH}")
        return report(errors)

    with CATALOG_PATH.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames != EXPECTED_HEADER:
            errors.append(
                "catalog.csv header does not match expected onboarding contract"
            )
        rows = list(reader)

    if not rows:
        errors.append("catalog.csv does not contain any rows")

    area_readmes: set[Path] = set()
    for row_number, row in enumerate(rows, start=2):
        area_id = row["area_id"].strip()
        intended_use = row["intended_use"].strip()
        status = row["status"].strip()
        terrain_type = row["terrain_type"].strip()
        local_files = row["local_files"].strip()

        if not area_id:
            errors.append(f"catalog.csv row {row_number}: area_id must not be empty")
        if intended_use not in ALLOWED_INTENDED_USE:
            errors.append(
                f"catalog.csv row {row_number}: invalid intended_use '{intended_use}'"
            )
        if status not in ALLOWED_STATUS:
            errors.append(
                f"catalog.csv row {row_number}: invalid status '{status}'"
            )
        if terrain_type not in ALLOWED_TERRAIN_TYPE:
            errors.append(
                f"catalog.csv row {row_number}: invalid terrain_type '{terrain_type}'"
            )
        if local_files:
            referenced_path = REPO_ROOT / local_files
            if not referenced_path.exists():
                errors.append(
                    f"catalog.csv row {row_number}: referenced local_files path is missing: {local_files}"
                )
            elif referenced_path.name == "README.md":
                area_readmes.add(referenced_path)

    for readme_path in sorted(area_readmes):
        validate_area_readme(readme_path, errors)

    return report(errors)


def validate_area_readme(readme_path: Path, errors: list[str]) -> None:
    content = readme_path.read_text(encoding="utf-8")
    for section in REQUIRED_AREA_SECTIONS:
        if section not in content:
            errors.append(
                f"{readme_path.relative_to(REPO_ROOT)} is missing required section: {section}"
            )


def report(errors: list[str]) -> int:
    if not errors:
        print("terrain_library_validation=ok")
        return 0

    print("terrain_library_validation=failed", file=sys.stderr)
    for error in errors:
        print(f"- {error}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
