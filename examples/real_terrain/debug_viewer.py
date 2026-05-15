#!/usr/bin/env python3

from __future__ import annotations

import argparse
import csv
import math
import os
import re
import subprocess
import tempfile
import tkinter as tk
from dataclasses import dataclass
from pathlib import Path
from tkinter import ttk


SNAPSHOT_RE = re.compile(r"^(?P<stem>.+)_step(?P<step>\d+)_t(?P<seconds>\d+)s\.csv$")


@dataclass(frozen=True)
class RasterFrame:
    path: Path
    rows: int
    cols: int
    metadata: dict[str, str]
    elevation: list[float]
    water_depth: list[float] | None
    surface_height: list[float] | None


OVERLAY_MAX_CELLS = 100
OVERLAY_MIN_CELL_WIDTH = 55.0
OVERLAY_MIN_CELL_HEIGHT = 28.0


def default_terrain_export_binary() -> Path:
    env_path = os.environ.get("FLOODSIM_TERRAIN_DEBUG_EXPORT")
    if env_path:
        return Path(env_path)

    repo_root = Path(__file__).resolve().parents[2]
    return repo_root / "build" / "floodsim_terrain_debug_export"


def parse_floodsim_csv(path: str | Path) -> RasterFrame:
    csv_path = Path(path)
    metadata: dict[str, str] = {}
    elevation: list[float] = []
    water_depth: list[float] = []
    surface_height: list[float] = []
    header_seen = False

    with csv_path.open("r", encoding="utf-8", newline="") as handle:
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
                    raise ValueError(f"Unexpected FloodSim CSV header in {csv_path}")
                header_seen = True
                continue

            row = next(csv.reader([line]))
            if len(row) != 5:
                raise ValueError(f"Unexpected FloodSim CSV row width in {csv_path}")
            elevation.append(float(row[2]))
            water_depth.append(float(row[3]))
            surface_height.append(float(row[4]))

    if not header_seen:
        raise ValueError(f"Missing FloodSim CSV header in {csv_path}")

    rows = int(metadata["rows"])
    cols = int(metadata["cols"])
    expected_cells = rows * cols
    if len(elevation) != expected_cells:
        raise ValueError(
            f"Expected {expected_cells} cells from metadata in {csv_path}, found {len(elevation)}"
        )

    return RasterFrame(
        path=csv_path,
        rows=rows,
        cols=cols,
        metadata=metadata,
        elevation=elevation,
        water_depth=water_depth,
        surface_height=surface_height,
    )


def parse_esri_ascii_grid(path: str | Path) -> RasterFrame:
    grid_path = Path(path)
    metadata: dict[str, str] = {}

    with grid_path.open("r", encoding="utf-8") as handle:
        header_keys = ("ncols", "nrows", "xllcorner", "yllcorner", "cellsize", "NODATA_value")
        for key in header_keys:
            line = handle.readline()
            if not line:
                raise ValueError(f"Incomplete ASCII grid header in {grid_path}")
            parsed_key, parsed_value = line.strip().split(maxsplit=1)
            if parsed_key != key:
                raise ValueError(f"Unexpected ASCII grid header key '{parsed_key}' in {grid_path}")
            metadata[key] = parsed_value

        cols = int(metadata["ncols"])
        rows = int(metadata["nrows"])
        nodata_value = float(metadata["NODATA_value"])
        elevation: list[float] = []
        for raw_line in handle:
            stripped = raw_line.strip()
            if not stripped:
                continue
            for token in stripped.split():
                value = float(token)
                elevation.append(math.nan if value == nodata_value else value)

    expected_cells = rows * cols
    if len(elevation) != expected_cells:
        raise ValueError(
            f"Expected {expected_cells} cells from ASCII grid metadata in {grid_path}, found {len(elevation)}"
        )

    view_metadata = {
        "rows": str(rows),
        "cols": str(cols),
        "cell_size_m": metadata["cellsize"],
        "origin_x_m": metadata["xllcorner"],
        "origin_y_m": str(float(metadata["yllcorner"]) + float(metadata["cellsize"]) * rows),
        "source_format": "esri_ascii_grid",
    }
    return RasterFrame(
        path=grid_path,
        rows=rows,
        cols=cols,
        metadata=view_metadata,
        elevation=elevation,
        water_depth=None,
        surface_height=None,
    )


def discover_snapshot_series(path: str | Path) -> list[Path]:
    csv_path = Path(path)
    match = SNAPSHOT_RE.match(csv_path.name)
    if match:
        final_stem = match.group("stem")
        final_path = csv_path.with_name(final_stem + ".csv")
    else:
        final_stem = csv_path.stem
        final_path = csv_path

    snapshot_paths = sorted(
        final_path.parent.glob(final_stem + "_step*_t*s.csv"),
        key=lambda candidate: candidate.name,
    )
    if final_path.exists():
        snapshot_paths.append(final_path)
    return snapshot_paths


def convert_raster_to_viewer_csv(
    raster_path: str | Path,
    terrain_export_binary: str | Path | None = None,
) -> Path:
    raster_path = Path(raster_path)
    export_binary = Path(terrain_export_binary) if terrain_export_binary else default_terrain_export_binary()
    if not export_binary.exists():
        raise FileNotFoundError(
            f"Terrain export helper not found at {export_binary}. "
            "Set FLOODSIM_TERRAIN_DEBUG_EXPORT or pass --terrain-export-binary."
        )

    temp_dir = Path(tempfile.mkdtemp(prefix="floodsim_viewer_"))
    output_csv_path = temp_dir / (raster_path.stem + "_terrain_view.csv")
    subprocess.run(
        [str(export_binary), str(raster_path), str(output_csv_path)],
        check=True,
        capture_output=True,
        text=True,
    )
    return output_csv_path


def load_frames(
    paths: list[str],
    terrain_export_binary: str | Path | None = None,
) -> list[RasterFrame]:
    if not paths:
        raise ValueError("No input paths provided")

    parsed_paths = [Path(path) for path in paths]
    if len(parsed_paths) == 1 and parsed_paths[0].suffix.lower() == ".csv":
        candidate_paths = discover_snapshot_series(parsed_paths[0])
        if not candidate_paths:
            raise FileNotFoundError(
                f"No FloodSim CSV frames found for {parsed_paths[0]}. "
                "Expected the final export CSV itself or matching snapshot CSVs beside it."
            )
    else:
        candidate_paths = parsed_paths

    frames: list[RasterFrame] = []
    for path in candidate_paths:
        if not path.exists():
            raise FileNotFoundError(f"Viewer input not found: {path}")
        suffix = path.suffix.lower()
        if suffix == ".csv":
            frames.append(parse_floodsim_csv(path))
        elif suffix == ".asc":
            frames.append(parse_esri_ascii_grid(path))
        elif suffix in {".tif", ".tiff"}:
            frames.append(parse_floodsim_csv(convert_raster_to_viewer_csv(path, terrain_export_binary)))
        else:
            raise ValueError(f"Unsupported viewer input format: {path}")
    return frames


def format_metadata(frame: RasterFrame, layer_name: str, frame_index: int, frame_count: int) -> str:
    scenario_name = frame.metadata.get("scenario_name", "none")
    total_duration = frame.metadata.get("total_duration_seconds", "n/a")
    return (
        f"frame={frame_index + 1}/{frame_count}  "
        f"layer={layer_name}  "
        f"scenario={scenario_name}  "
        f"rows={frame.rows} cols={frame.cols}  "
        f"total_duration_seconds={total_duration}  "
        f"path={frame.path}"
    )


def format_value(value: float) -> str:
    if math.isnan(value):
        return "nodata"
    return f"{value:.3f}"


def should_draw_value_overlay(rows: int, cols: int, cell_width: float, cell_height: float) -> bool:
    return (
        rows * cols <= OVERLAY_MAX_CELLS
        and cell_width >= OVERLAY_MIN_CELL_WIDTH
        and cell_height >= OVERLAY_MIN_CELL_HEIGHT
    )


def text_color_for_hex(fill_color: str) -> str:
    red = int(fill_color[1:3], 16)
    green = int(fill_color[3:5], 16)
    blue = int(fill_color[5:7], 16)
    luminance = 0.299 * red + 0.587 * green + 0.114 * blue
    return "#111111" if luminance >= 150.0 else "#f2f2f2"


def color_for_value(value: float, min_value: float, max_value: float, palette: str) -> str:
    if math.isnan(value):
        return "#202020"
    if max_value <= min_value:
        normalized = 0.0
    else:
        normalized = (value - min_value) / (max_value - min_value)
    normalized = min(max(normalized, 0.0), 1.0)

    if palette == "terrain":
        shade = int(40 + normalized * 180)
        return f"#{shade:02x}{shade:02x}{shade:02x}"
    if palette == "water":
        blue = int(80 + normalized * 175)
        green = int(40 + normalized * 90)
        return f"#10{green:02x}{blue:02x}"
    # surface
    red = int(60 + normalized * 150)
    green = int(40 + normalized * 110)
    blue = int(70 + normalized * 120)
    return f"#{red:02x}{green:02x}{blue:02x}"


class DebugViewer:
    def __init__(self, frames: list[RasterFrame]) -> None:
        self.frames = frames
        self.frame_index = 0
        self.layer = "water_depth" if frames[0].water_depth is not None else "elevation"
        self.hovered_cell: tuple[int, int] | None = None

        self.root = tk.Tk()
        self.root.title("FloodSim Debug Viewer")
        self.root.geometry("900x780")

        controls = ttk.Frame(self.root, padding=8)
        controls.pack(fill="x")

        ttk.Button(controls, text="Prev", command=self.prev_frame).pack(side="left")
        ttk.Button(controls, text="Next", command=self.next_frame).pack(side="left", padx=(4, 16))

        ttk.Label(controls, text="Layer").pack(side="left")
        self.layer_var = tk.StringVar(value=self.layer)
        layer_menu = ttk.OptionMenu(
            controls,
            self.layer_var,
            self.layer,
            "elevation",
            "water_depth",
            "surface_height",
            command=self.on_layer_change,
        )
        layer_menu.pack(side="left", padx=(4, 0))

        self.info_var = tk.StringVar()
        ttk.Label(self.root, textvariable=self.info_var, padding=(8, 0, 8, 8), wraplength=860).pack(fill="x")
        self.scale_var = tk.StringVar()
        ttk.Label(self.root, textvariable=self.scale_var, padding=(8, 0, 8, 6), wraplength=860).pack(fill="x")
        self.detail_var = tk.StringVar(value="hover a cell to inspect exact values")
        ttk.Label(self.root, textvariable=self.detail_var, padding=(8, 0, 8, 8), wraplength=860).pack(fill="x")

        self.canvas = tk.Canvas(self.root, background="#111111", highlightthickness=0)
        self.canvas.pack(fill="both", expand=True, padx=8, pady=(0, 8))

        self.root.bind("<Left>", lambda _event: self.prev_frame())
        self.root.bind("<Right>", lambda _event: self.next_frame())
        self.root.bind("<Configure>", lambda _event: self.draw())
        self.canvas.bind("<Motion>", self.on_mouse_move)
        self.canvas.bind("<Leave>", self.on_mouse_leave)

        self.draw()

    def dataset_for_layer(self, frame: RasterFrame) -> tuple[list[float], str]:
        if self.layer == "elevation":
            return frame.elevation, "terrain"
        if self.layer == "water_depth" and frame.water_depth is not None:
            return frame.water_depth, "water"
        if self.layer == "surface_height" and frame.surface_height is not None:
            return frame.surface_height, "surface"
        return frame.elevation, "terrain"

    def on_layer_change(self, _value: str | None = None) -> None:
        self.layer = self.layer_var.get()
        self.draw()

    def prev_frame(self) -> None:
        self.frame_index = (self.frame_index - 1) % len(self.frames)
        self.draw()

    def next_frame(self) -> None:
        self.frame_index = (self.frame_index + 1) % len(self.frames)
        self.draw()

    def on_mouse_move(self, event: tk.Event[tk.Misc]) -> None:
        frame = self.frames[self.frame_index]
        canvas_width = max(self.canvas.winfo_width(), 200)
        canvas_height = max(self.canvas.winfo_height(), 200)
        cell_width = canvas_width / frame.cols
        cell_height = canvas_height / frame.rows
        col = min(max(int(event.x / cell_width), 0), frame.cols - 1)
        row = min(max(int(event.y / cell_height), 0), frame.rows - 1)
        hovered = (row, col)
        if hovered != self.hovered_cell:
            self.hovered_cell = hovered
            self.update_detail_label()

    def on_mouse_leave(self, _event: tk.Event[tk.Misc]) -> None:
        self.hovered_cell = None
        self.update_detail_label()

    def update_detail_label(self) -> None:
        if self.hovered_cell is None:
            self.detail_var.set("hover a cell to inspect exact values")
            return

        frame = self.frames[self.frame_index]
        row, col = self.hovered_cell
        index = row * frame.cols + col
        water_depth = frame.water_depth[index] if frame.water_depth is not None else math.nan
        surface_height = frame.surface_height[index] if frame.surface_height is not None else math.nan
        self.detail_var.set(
            f"cell=({row},{col})  "
            f"elevation_m={format_value(frame.elevation[index])}  "
            f"water_depth_m={format_value(water_depth)}  "
            f"surface_height_m={format_value(surface_height)}"
        )

    def draw(self) -> None:
        frame = self.frames[self.frame_index]
        dataset, palette = self.dataset_for_layer(frame)
        finite_values = [value for value in dataset if not math.isnan(value)]
        min_value = min(finite_values) if finite_values else 0.0
        max_value = max(finite_values) if finite_values else 0.0

        self.canvas.delete("all")
        canvas_width = max(self.canvas.winfo_width(), 200)
        canvas_height = max(self.canvas.winfo_height(), 200)
        cell_width = canvas_width / frame.cols
        cell_height = canvas_height / frame.rows
        draw_overlay = should_draw_value_overlay(frame.rows, frame.cols, cell_width, cell_height)

        for row in range(frame.rows):
            for col in range(frame.cols):
                index = row * frame.cols + col
                value = dataset[index]
                x0 = col * cell_width
                y0 = row * cell_height
                x1 = (col + 1) * cell_width
                y1 = (row + 1) * cell_height
                fill_color = color_for_value(value, min_value, max_value, palette)
                self.canvas.create_rectangle(
                    x0,
                    y0,
                    x1,
                    y1,
                    fill=fill_color,
                    outline="#1e1e1e",
                )
                if draw_overlay:
                    self.canvas.create_text(
                        (x0 + x1) / 2.0,
                        (y0 + y1) / 2.0,
                        text=format_value(value),
                        fill=text_color_for_hex(fill_color),
                        font=("TkDefaultFont", 10),
                    )

        self.info_var.set(format_metadata(frame, self.layer, self.frame_index, len(self.frames)))
        self.scale_var.set(
            f"scale={self.layer}  min={format_value(min_value)}  max={format_value(max_value)}  "
            f"mode=dynamic-per-frame"
        )
        self.update_detail_label()

    def run(self) -> None:
        self.root.mainloop()


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Local FloodSim debugging viewer for FloodSim CSV exports, snapshot series, "
            "and ESRI ASCII terrain rasters."
        )
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help=(
            "One or more FloodSim CSV exports or ESRI ASCII grids. If one final CSV is "
            "provided, matching snapshot CSVs beside it are discovered automatically. "
            "GeoTIFF inputs are converted through the local terrain-export helper."
        ),
    )
    parser.add_argument(
        "--terrain-export-binary",
        default=str(default_terrain_export_binary()),
        help="Path to the floodsim_terrain_debug_export helper used for GeoTIFF inputs.",
    )
    args = parser.parse_args()

    try:
        DebugViewer(load_frames(args.inputs, args.terrain_export_binary)).run()
    except (FileNotFoundError, ValueError, subprocess.CalledProcessError, tk.TclError) as error:
        parser.exit(1, f"error: {error}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
