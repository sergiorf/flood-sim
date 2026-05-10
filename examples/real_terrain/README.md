# Real Terrain Example

This example is the first end-to-end Phase 2 workflow in the repository. It
loads a real-format terrain raster through GDAL, maps the imported terrain into
the simulation grid, runs a short rainfall simulation, and exports the final
grid state as CSV.

The committed input raster is intentionally tiny:

- file: `examples/real_terrain/data/sample_dem.tif`
- size: `5 x 5`
- cell size: `2m`
- CRS: `EPSG:31370`
- one nodata cell to exercise the imported-domain rules

Build and run from the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output.csv
```

That default invocation remains the canonical reproducible smoke-test path. It
uses:

- rainfall intensity: `0.012 m/hour`
- step duration: `300 seconds`
- number of steps: `12`
- boundary mode: `Closed`

Those run fields now flow through one narrow `ScenarioConfig` inside the
example so default runs, CLI overrides, and later named scenarios can share the
same validation path.

You can run one of the documented named scenarios through `--scenario`:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_intense_short.csv \
  --scenario intense_short
```

Current named scenarios:

- `baseline`: `0.012 m/hour` for `12` steps of `300 seconds`, representing a moderate one-hour event at `12 mm/hour`
- `intense_short`: `0.030 m/hour` for `6` steps of `300 seconds`, representing a short `30` minute burst at `30 mm/hour`
- `long_moderate`: `0.008 m/hour` for `36` steps of `300 seconds`, representing a longer `3` hour event at `8 mm/hour`

These presets are intended to be plausible screening events for repeatable
comparison, not calibrated local storm models. They are useful because they
keep the repository talking about the same runs consistently while the
underlying hydrology is still intentionally simple.

You can also override the rainfall and time-step settings through a small CLI:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output_heavier_rain.csv \
  --scenario baseline \
  --rainfall-intensity-m-per-hour 0.020 \
  --time-step-seconds 600 \
  --steps 4
```

If a named scenario and explicit numeric flags are both provided, the numeric
flags win. The example prints the selected scenario name plus whether CLI
overrides were applied so those combinations are never silent.

You can clip a smaller pixel window from a larger source raster when you want a
repeatable real-area scenario without preprocessing a separate file first:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_window.csv \
  --window-row-offset 1 \
  --window-col-offset 1 \
  --window-rows 3 \
  --window-cols 2
```

Supported options:

- `--scenario <name>`: load one documented rainfall preset: `baseline`, `intense_short`, or `long_moderate`
- `--rainfall-intensity-m-per-hour <value>`: uniform rainfall intensity in meters per hour, default `0.012`
- `--time-step-seconds <value>`: simulation step duration in seconds, default `300`
- `--steps <count>`: number of simulation steps to run, default `12`
- `--window-row-offset <value>`: top-row index of a clipped terrain window, default `0`
- `--window-col-offset <value>`: left-column index of a clipped terrain window, default `0`
- `--window-rows <value>`: number of rows in the clipped terrain window
- `--window-cols <value>`: number of columns in the clipped terrain window

Invalid values fail clearly. Rainfall intensity must be non-negative, and both
the time step and step count must be positive. If any terrain-window option is
used, both `--window-rows` and `--window-cols` are required, and the requested
window must stay within the source raster bounds. Scenario validation is kept
local to that `ScenarioConfig` construction instead of being spread across the
simulation setup path.

The example prints a short load and simulation summary, then writes the same
CSV contract used elsewhere in the repository with added georeferencing
metadata from the source raster:

```text
scenario_name=baseline scenario_source=direct_cli_or_default
ingestion_report source_rows=5 source_cols=5 loaded_rows=5 loaded_cols=5 clipped_cells=0 invalid_cells=1 nodata_metadata_present=true nan_cells=0 nodata_status=band_metadata_applied
# floodsim_csv_version,1
# rows,5
# cols,5
# cell_size_m,2.000000
# origin_x_m,154320.000000
# origin_y_m,171205.000000
# crs_id,EPSG:31370
row,col,elevation_m,water_depth_m,surface_height_m
...
```

You can inspect the exported CSV with the existing consumer example:

```bash
python3 examples/simple_grid/inspect_export.py real_terrain_output.csv
```

What this example proves:

- the GDAL loader can read a committed GeoTIFF from disk
- the same loader can clip a smaller pixel window while preserving shifted origin metadata
- imported nodata is preserved as out-of-domain cells
- the loader emits a machine-readable ingestion summary for nodata and clipping review
- the simulation can run on the imported terrain
- the final state can be exported with origin / CRS metadata for later inspection and visualization work

What it does not prove yet:

- map rendering
- reprojection workflows
- large-area performance
- urban drainage, buildings, or calibration
