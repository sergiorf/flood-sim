from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


DATA_DIR = Path(__file__).resolve().parent / "data"


@dataclass(frozen=True)
class RealTerrainFixture:
    name: str
    terrain_path: Path
    scenario_args: tuple[str, ...]
    intent: str
    expected_stdout_fragments: tuple[str, ...]
    expected_metadata: dict[str, str]
    expected_row_count: int
    expected_deepest_row: str
    expected_deepest_col: str


REAL_TERRAIN_FIXTURES: tuple[RealTerrainFixture, ...] = (
    RealTerrainFixture(
        name="nodata_bowl",
        terrain_path=DATA_DIR / "sample_dem.tif",
        scenario_args=(),
        intent=(
            "Exercises one out-of-domain nodata cell on the downslope side of the "
            "terrain while preserving CRS-aware export metadata."
        ),
        expected_stdout_fragments=(
            "scenario_name=baseline scenario_source=direct_cli_or_default",
            "boundary_mode=open",
            "runoff_coefficient=1.000000",
            (
                "ingestion_report source_rows=5 source_cols=5 loaded_rows=5 loaded_cols=5 "
                "clipped_cells=0 invalid_cells=1 nodata_metadata_present=true nan_cells=0 "
                "nodata_status=band_metadata_applied"
            ),
        ),
        expected_metadata={
            "floodsim_csv_version": "1",
            "rows": "5",
            "cols": "5",
            "cell_size_m": "2.000000",
            "scenario_name": "baseline",
            "boundary_mode": "open",
            "rainfall_intensity_m_per_hour": "0.012000",
            "runoff_coefficient": "1.000000",
            "initial_loss_m": "0.000000",
            "time_step_seconds": "300.000000",
            "total_duration_seconds": "3600.000000",
            "origin_x_m": "154320.000000",
            "origin_y_m": "171205.000000",
            "crs_id": "EPSG:31370",
        },
        expected_row_count=25,
        expected_deepest_row="2",
        expected_deepest_col="2",
    ),
    RealTerrainFixture(
        name="drainage_slope",
        terrain_path=DATA_DIR / "drainage_slope.asc",
        scenario_args=(),
        intent=(
            "Exercises a fully valid monotonic slope so regression tests can verify "
            "a simple deterministic drainage direction without nodata behavior."
        ),
        expected_stdout_fragments=(
            "scenario_name=baseline scenario_source=direct_cli_or_default",
            "boundary_mode=open",
            "runoff_coefficient=1.000000",
            (
                "ingestion_report source_rows=4 source_cols=4 loaded_rows=4 loaded_cols=4 "
                "clipped_cells=0 invalid_cells=0 nodata_metadata_present=true nan_cells=0 "
                "nodata_status=band_metadata_applied"
            ),
        ),
        expected_metadata={
            "floodsim_csv_version": "1",
            "rows": "4",
            "cols": "4",
            "cell_size_m": "5.000000",
            "scenario_name": "baseline",
            "boundary_mode": "open",
            "rainfall_intensity_m_per_hour": "0.012000",
            "runoff_coefficient": "1.000000",
            "initial_loss_m": "0.000000",
            "time_step_seconds": "300.000000",
            "total_duration_seconds": "3600.000000",
            "origin_x_m": "5000.000000",
            "origin_y_m": "1020.000000",
        },
        expected_row_count=16,
        expected_deepest_row="3",
        expected_deepest_col="3",
    ),
    RealTerrainFixture(
        name="flat_pond",
        terrain_path=DATA_DIR / "flat_pond.asc",
        scenario_args=(),
        intent=(
            "Exercises a flat basin-like depression where the main question is "
            "how much water ponds in place rather than how fast it drains away."
        ),
        expected_stdout_fragments=(
            "scenario_name=baseline scenario_source=direct_cli_or_default",
            "boundary_mode=open",
            "runoff_coefficient=1.000000",
            (
                "ingestion_report source_rows=5 source_cols=5 loaded_rows=5 loaded_cols=5 "
                "clipped_cells=0 invalid_cells=0 nodata_metadata_present=true nan_cells=0 "
                "nodata_status=band_metadata_applied"
            ),
        ),
        expected_metadata={
            "floodsim_csv_version": "1",
            "rows": "5",
            "cols": "5",
            "cell_size_m": "5.000000",
            "scenario_name": "baseline",
            "boundary_mode": "open",
            "rainfall_intensity_m_per_hour": "0.012000",
            "runoff_coefficient": "1.000000",
            "initial_loss_m": "0.000000",
            "time_step_seconds": "300.000000",
            "total_duration_seconds": "3600.000000",
            "origin_x_m": "8000.000000",
            "origin_y_m": "2025.000000",
        },
        expected_row_count=25,
        expected_deepest_row="2",
        expected_deepest_col="2",
    ),
    RealTerrainFixture(
        name="edge_notch",
        terrain_path=DATA_DIR / "edge_notch.asc",
        scenario_args=(),
        intent=(
            "Exercises an edge-drainage case with two nodata cells cut out on the "
            "downslope boundary so outflow and valid-domain handling interact."
        ),
        expected_stdout_fragments=(
            "scenario_name=baseline scenario_source=direct_cli_or_default",
            "boundary_mode=open",
            "runoff_coefficient=1.000000",
            (
                "ingestion_report source_rows=5 source_cols=5 loaded_rows=5 loaded_cols=5 "
                "clipped_cells=0 invalid_cells=2 nodata_metadata_present=true nan_cells=0 "
                "nodata_status=band_metadata_applied"
            ),
        ),
        expected_metadata={
            "floodsim_csv_version": "1",
            "rows": "5",
            "cols": "5",
            "cell_size_m": "5.000000",
            "scenario_name": "baseline",
            "boundary_mode": "open",
            "rainfall_intensity_m_per_hour": "0.012000",
            "runoff_coefficient": "1.000000",
            "initial_loss_m": "0.000000",
            "time_step_seconds": "300.000000",
            "total_duration_seconds": "3600.000000",
            "origin_x_m": "12000.000000",
            "origin_y_m": "4025.000000",
        },
        expected_row_count=25,
        expected_deepest_row="0",
        expected_deepest_col="3",
    ),
    RealTerrainFixture(
        name="urban_block",
        terrain_path=DATA_DIR / "urban_block.asc",
        scenario_args=(),
        intent=(
            "Exercises a slightly larger barrier-like layout where raised blocks "
            "split local routing and create multiple urban-ish flow paths."
        ),
        expected_stdout_fragments=(
            "scenario_name=baseline scenario_source=direct_cli_or_default",
            "boundary_mode=open",
            "runoff_coefficient=1.000000",
            (
                "ingestion_report source_rows=6 source_cols=6 loaded_rows=6 loaded_cols=6 "
                "clipped_cells=0 invalid_cells=0 nodata_metadata_present=true nan_cells=0 "
                "nodata_status=band_metadata_applied"
            ),
        ),
        expected_metadata={
            "floodsim_csv_version": "1",
            "rows": "6",
            "cols": "6",
            "cell_size_m": "4.000000",
            "scenario_name": "baseline",
            "boundary_mode": "open",
            "rainfall_intensity_m_per_hour": "0.012000",
            "runoff_coefficient": "1.000000",
            "initial_loss_m": "0.000000",
            "time_step_seconds": "300.000000",
            "total_duration_seconds": "3600.000000",
            "origin_x_m": "20000.000000",
            "origin_y_m": "6024.000000",
        },
        expected_row_count=36,
        expected_deepest_row="2",
        expected_deepest_col="2",
    ),
    RealTerrainFixture(
        name="split_basin",
        terrain_path=DATA_DIR / "split_basin.asc",
        scenario_args=(),
        intent=(
            "Exercises a dual-depression catchment with an interior saddle so "
            "regression coverage includes split retention patterns rather than "
            "only one dominant pond or one dominant edge outlet."
        ),
        expected_stdout_fragments=(
            "scenario_name=baseline scenario_source=direct_cli_or_default",
            "boundary_mode=open",
            "runoff_coefficient=1.000000",
            (
                "ingestion_report source_rows=6 source_cols=6 loaded_rows=6 loaded_cols=6 "
                "clipped_cells=0 invalid_cells=0 nodata_metadata_present=true nan_cells=0 "
                "nodata_status=band_metadata_applied"
            ),
        ),
        expected_metadata={
            "floodsim_csv_version": "1",
            "rows": "6",
            "cols": "6",
            "cell_size_m": "4.000000",
            "scenario_name": "baseline",
            "boundary_mode": "open",
            "rainfall_intensity_m_per_hour": "0.012000",
            "runoff_coefficient": "1.000000",
            "initial_loss_m": "0.000000",
            "time_step_seconds": "300.000000",
            "total_duration_seconds": "3600.000000",
            "origin_x_m": "26000.000000",
            "origin_y_m": "7024.000000",
        },
        expected_row_count=36,
        expected_deepest_row="2",
        expected_deepest_col="2",
    ),
)
