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

The repository also carries a small regression-fixture set for this example:

- `sample_dem.tif`: canonical nodata-aware basin fixture with CRS metadata preserved in exports
- `drainage_slope.asc`: monotonic fully valid slope fixture for deterministic drainage-direction regression coverage
- `flat_pond.asc`: flat ponding-prone basin fixture for retained-water behavior
- `edge_notch.asc`: clipped-edge nodata notch fixture for domain and outflow interaction
- `urban_block.asc`: slightly larger barrier-style fixture for split routing and future viewer/debug work

Those fixtures are example and test assets, not engine-level scenario
definitions. The simulation core still receives only validated terrain data
plus the explicit rainfall and step settings built by the example.
The example implementation itself is split the same way: a small reusable C++
workflow helper owns scenario parsing, validation, execution, reporting, and
CSV orchestration, while `main.cpp` stays a thin CLI wrapper.

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
- boundary mode: `Open`

For conservative comparisons or toy-style retention checks, you can still
force the older closed-edge behavior explicitly:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_closed_boundary.csv \
  --boundary-mode closed
```

The default `open` mode is intentionally simple. It lets edge cells discharge
part of their water out of the raster across missing orthogonal neighbors at
the raster perimeter. That is often a more practical approximation for clipped
real terrain than `closed` mode, but it is still not a drainage-network or
calibrated outflow model.

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

You can also define scenarios outside the binary through a narrow CSV contract:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_from_file.csv \
  --scenario-file examples/real_terrain/data/sample_single_scenario.csv
```

The current scenario-file header is fixed and intentionally small:

```text
scenario_name,rainfall_intensity_m_per_hour,runoff_coefficient,time_step_seconds,steps,boundary_mode
```

Each non-empty row defines one scenario. The committed sample file looks like:

```text
scenario_name,rainfall_intensity_m_per_hour,runoff_coefficient,time_step_seconds,steps,boundary_mode
reviewed_screening,0.012000,1.000000,300.000000,12,open
```

Field meanings:

- `scenario_name`: identifier written into reports and export metadata
- `rainfall_intensity_m_per_hour`: uniform rainfall intensity in meters per hour
- `runoff_coefficient`: retained rainfall fraction in `[0, 1]`
- `time_step_seconds`: duration of one step in seconds
- `steps`: positive integer number of steps
- `boundary_mode`: `open` or `closed`

Invalid files fail clearly for missing headers, malformed rows, unknown
boundary modes, or invalid numeric bounds.

You can also run several named scenarios over the same clip in one invocation:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_batch.csv \
  --batch-scenarios baseline,intense_short,long_moderate
```

A multi-row scenario file uses the same batch/export path without requiring the
scenario names to stay compiled into the example:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_file_batch.csv \
  --scenario-file examples/real_terrain/data/sample_scenarios.csv
```

That batch path keeps the interface intentionally narrow:

- the scenario list is a comma-separated set of the documented preset names
- or a multi-row `--scenario-file` with the fixed header above
- the same terrain clip, boundary mode, runoff coefficient, and optional window apply to every scenario in the batch
- the positional output path becomes a deterministic base name, expanded into:
- `real_terrain_batch_baseline.csv`
- `real_terrain_batch_intense_short.csv`
- `real_terrain_batch_long_moderate.csv`
- `real_terrain_batch_comparison.csv`

Each successful scenario still prints the same per-run summary and writes the
same self-describing CSV metadata contract as single-scenario mode. If one
scenario fails, the example reports `scenario_failed ...` with the scenario
name and target output path so the failure is explicit.

The batch mode also writes one compact comparison artifact beside those
per-scenario CSVs. The current contract is a narrow CSV table with one row per
scenario and these columns:

- `scenario_name`
- `boundary_mode`
- `rainfall_intensity_m_per_hour`
- `runoff_coefficient`
- `time_step_seconds`
- `steps`
- `total_water_depth_m`
- `wet_cells`
- `max_water_depth_m`
- `deepest_row`
- `deepest_col`
- `output_csv`

You can also override the rainfall and time-step settings through a small CLI:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output_heavier_rain.csv \
  --scenario baseline \
  --rainfall-intensity-m-per-hour 0.020 \
  --runoff-coefficient 0.5 \
  --time-step-seconds 600 \
  --steps 4
```

If a named scenario and explicit numeric flags are both provided, the numeric
flags win. The example prints the selected scenario name plus whether CLI
overrides were applied so those combinations are never silent.

The same CLI can apply a simple runoff-retention control through
`--runoff-coefficient`. A value of `1.0` means all rainfall becomes immediate
surface water in the current raster model. Lower values approximate simple
losses such as infiltration or local retention before water appears as ponded
surface depth.

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

You can also export a small number of intermediate runoff snapshots during a
run:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output.csv \
  --snapshot-every-steps 4
```

With the default baseline scenario (`12` steps of `300` seconds), that writes:

- `real_terrain_output.csv` for the final state at step `12`
- `real_terrain_output_step0004_t1200s.csv`
- `real_terrain_output_step0008_t2400s.csv`

The snapshot path contract is intentionally simple and deterministic:

- base output stem from the positional CSV path
- `_stepNNNN` using zero-padded completed-step count
- `_t<seconds>s` using elapsed simulation seconds
- original `.csv` suffix

Snapshots are intermediate-only. If a selected snapshot step would equal the
final export step, the example keeps only the normal final output CSV rather
than writing a duplicate snapshot file.

Supported options:

- `--scenario <name>`: load one documented rainfall preset: `baseline`, `intense_short`, or `long_moderate`
- `--batch-scenarios <name1,name2,...>`: run several documented scenario presets in one invocation and derive one output CSV per scenario from the positional output path
- `--scenario-file <path.csv>`: load one or more external scenario definitions from the fixed CSV contract above
- `--snapshot-every-steps <count>`: write one intermediate snapshot CSV every N completed steps, excluding the final output step
- `--boundary-mode <closed|open>`: choose whether raster edges trap water or allow edge outflow, default `open` for the real-terrain workflow
- `--rainfall-intensity-m-per-hour <value>`: uniform rainfall intensity in meters per hour, default `0.012`
- `--runoff-coefficient <value>`: fraction of rainfall retained as immediate surface runoff, default `1.0`
- `--time-step-seconds <value>`: simulation step duration in seconds, default `300`
- `--steps <count>`: number of simulation steps to run, default `12`
- `--window-row-offset <value>`: top-row index of a clipped terrain window, default `0`
- `--window-col-offset <value>`: left-column index of a clipped terrain window, default `0`
- `--window-rows <value>`: number of rows in the clipped terrain window
- `--window-cols <value>`: number of columns in the clipped terrain window

Invalid values fail clearly. Rainfall intensity must be non-negative, and both
the time step and step count must be positive. If any terrain-window option is
used, both `--window-rows` and `--window-cols` are required, and the requested
window must stay within the source raster bounds. `--scenario` and
`--batch-scenarios` are mutually exclusive, and both are also mutually
exclusive with `--scenario-file`. Snapshot intervals must be positive.
Scenario validation is kept local to that `ScenarioConfig` construction instead
of being spread across the simulation setup path.

The example prints a short load and simulation summary, then writes the same
CSV contract used elsewhere in the repository with added georeferencing
metadata from the source raster plus enough scenario identity to compare runs
without confusing them:

```text
scenario_name=baseline scenario_source=direct_cli_or_default
boundary_mode=open
runoff_coefficient=1.000000
ingestion_report source_rows=5 source_cols=5 loaded_rows=5 loaded_cols=5 clipped_cells=0 invalid_cells=1 nodata_metadata_present=true nan_cells=0 nodata_status=band_metadata_applied
# floodsim_csv_version,1
# rows,5
# cols,5
# cell_size_m,2.000000
# scenario_name,baseline
# boundary_mode,open
# rainfall_intensity_m_per_hour,0.012000
# runoff_coefficient,1.000000
# time_step_seconds,300.000000
# total_duration_seconds,3600.000000
# origin_x_m,154320.000000
# origin_y_m,171205.000000
# crs_id,EPSG:31370
row,col,elevation_m,water_depth_m,surface_height_m
...
```

It also prints one compact run summary for quick comparison before a map viewer
exists:

```text
summary_metrics wet_cells=24 max_water_depth_m=0.097056 deepest_row=2 deepest_col=2
```

Current summary fields:

- `wet_cells`: number of valid cells with water depth above zero
- `max_water_depth_m`: deepest water depth among valid wet cells
- `deepest_row` and `deepest_col`: raster coordinates of that deepest wet cell

These are raster-model summaries for screening and comparison. They are useful
for fast repeatable review, but they are not a substitute for calibrated
hydrology metrics.

The runoff coefficient belongs to that same category: it is a simple practical
approximation for MVP scenario screening, not a calibrated infiltration model.

The default open boundary is also a practical screening choice. For clipped
real-terrain rasters it avoids treating the raster edge like a retaining wall
by default. It is still a narrow approximation: water may leave only across
missing orthogonal neighbors at the raster edge, and it is not a calibrated
downstream boundary condition.

You can inspect the exported CSV with the existing consumer example:

```bash
python3 examples/simple_grid/inspect_export.py real_terrain_output.csv
```

## Canonical Comparison Walkthrough

This is the current canonical Phase 2 comparison walkthrough for the committed
real clip. It uses the checked-in `sample_dem.tif` fixture plus the named
scenario presets so another contributor can rerun the same comparison exactly.

Run the three documented scenarios from the repository root:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  fs029_baseline.csv

./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  fs029_intense_short.csv \
  --scenario intense_short

./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  fs029_long_moderate.csv \
  --scenario long_moderate
```

Each run writes one deterministic CSV artifact:

- `fs029_baseline.csv`
- `fs029_intense_short.csv`
- `fs029_long_moderate.csv`
- `fs029_comparison.csv` if you instead use the batch invocation with `--batch-scenarios baseline,intense_short,long_moderate`

Each CSV should carry the same terrain metadata and a scenario-specific
metadata preamble, including:

- `# scenario_name,...`
- `# boundary_mode,open`
- `# rainfall_intensity_m_per_hour,...`
- `# runoff_coefficient,1.000000`
- `# time_step_seconds,300.000000`
- `# total_duration_seconds,...`
- `# origin_x_m,154320.000000`
- `# origin_y_m,171205.000000`
- `# crs_id,EPSG:31370`

For quick terminal-side comparison, focus first on the printed
`summary_metrics` line from each run:

```text
baseline:
summary_metrics wet_cells=24 max_water_depth_m=0.096997 deepest_row=2 deepest_col=2
total_water_depth_m=0.287743

intense_short:
summary_metrics wet_cells=24 max_water_depth_m=0.067712 deepest_row=2 deepest_col=2
total_water_depth_m=0.359635

long_moderate:
summary_metrics wet_cells=24 max_water_depth_m=0.402130 deepest_row=2 deepest_col=2
total_water_depth_m=0.575294
```

One compact way to interpret those outputs:

- all three runs wet the same 24 valid cells on this tiny clip, so the comparison is about magnitude rather than footprint extent
- `intense_short` retains more total water than `baseline`, but its peak depth is shallower on this fixture because the shorter run leaves less time for sustained ponding at the deepest cell
- `long_moderate` produces the deepest and largest retained result here because its total rainfall duration is much longer

If you want to inspect the exported artifacts directly, use the existing CSV
consumer:

```bash
python3 examples/simple_grid/inspect_export.py fs029_baseline.csv
python3 examples/simple_grid/inspect_export.py fs029_intense_short.csv
python3 examples/simple_grid/inspect_export.py fs029_long_moderate.csv
```

If you want the same comparison captured as one machine-readable artifact
instead of reading three separate run summaries, run the batch variant:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  fs029.csv \
  --batch-scenarios baseline,intense_short,long_moderate
```

That invocation writes:

- `fs029_baseline.csv`
- `fs029_intense_short.csv`
- `fs029_long_moderate.csv`
- `fs029_comparison.csv`

The comparison CSV currently contains one deterministic row per scenario with
the summary values listed above. It is intended for manual inspection and
future scripting, not as a stable long-term analytics schema yet.

What this walkthrough proves today:

- the same committed real clip can be rerun reproducibly with multiple named scenarios
- the example prints enough scenario identity and summary information to compare runs without guessing which output came from which setup
- the CSV exports preserve enough metadata for manual review or later scripting

Modeling limits for this walkthrough:

- the scenarios are screening presets, not calibrated storms
- rainfall is still spatially uniform
- runoff loss is disabled in this canonical comparison path with `runoff_coefficient=1.0`
- open-boundary outflow at the raster edge is a narrow approximation, not a drainage-network or downstream boundary model
- these comparisons are useful for deterministic workflow review, not for real flood-risk claims

What this example proves:

- the GDAL loader can read a committed GeoTIFF from disk
- the same ingestion path can also read a tiny text-based raster fixture for regression coverage
- the same loader can clip a smaller pixel window while preserving shifted origin metadata
- imported nodata is preserved as out-of-domain cells
- the loader emits a machine-readable ingestion summary for nodata and clipping review
- the workflow can make the raster edge behavior explicit for clipped real-terrain runs
- the simulation can run on the imported terrain
- the final state can be exported with origin / CRS metadata for later inspection and visualization work

Why the fixtures exist:

- `sample_dem.tif` keeps regression coverage on nodata-domain handling and CRS-carrying exports
- `drainage_slope.asc` keeps regression coverage on a simple clear drainage pattern without nodata or clipping noise
- `flat_pond.asc` keeps regression coverage on center-ponding behavior in a mostly flat basin
- `edge_notch.asc` keeps regression coverage on downslope nodata-edge handling where clipped outflow is the main question
- `urban_block.asc` keeps regression coverage on a slightly larger blocked-routing pattern that is useful for future visualization and debugging

That split keeps fixture intent readable in tests and docs without pushing
test-specific case naming into the simulation engine or the example CLI.

What it does not prove yet:

- map rendering
- reprojection workflows
- large-area performance
- urban drainage, buildings, or calibration
