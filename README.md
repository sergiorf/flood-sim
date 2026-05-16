# FloodSim

FloodSim is an urban flood simulation product focused on rainfall-driven flooding over real city terrain. The long-term goal is climate-risk simulation for cities, but the first milestone is intentionally small: a clear, buildable raster-grid prototype that can evolve into a more capable simulation stack.

## Product vision

FloodSim aims to support workflows such as:

- loading terrain and urban surface data
- simulating rainfall accumulation and overland flow
- exporting flood-depth outputs for maps and downstream analysis
- comparing infrastructure and climate scenarios over time

## MVP scope

This repository starts with a simplified first model:

- C++20 simulation core
- raster/grid terrain representation
- uniform rainfall over a grid
- simple downhill neighbor flow
- lightweight tests
- Python placeholders for future preprocessing and DEM ingestion

This is not a certified hydrology or hydrodynamics engine. The MVP is designed for clarity, testability, and iterative improvement.

## Repository layout

```text
flood-sim/
  core/               C++ simulation library and tests
  data/scripts/       preprocessing and ingestion helpers
  docs/               architecture, model notes, roadmap
  examples/           runnable toy examples
  AGENTS.md           instructions for future Codex work
```

## Build

Requirements:

- CMake 3.20+
- a C++20 compiler such as GCC 11+, Clang 14+, or MSVC with C++20 support
- `doctest` if you want to build the C++ test suite
- GDAL if you want to build upcoming real-terrain ingestion support

Build commands:

```bash
cmake -S . -B build
cmake --build build
```

To disable GDAL-backed terrain-ingestion support explicitly:

```bash
cmake -S . -B build -DFLOODSIM_ENABLE_GDAL=OFF
```

To configure without building tests, use:

```bash
cmake -S . -B build -DFLOODSIM_BUILD_TESTS=OFF
```

The first GDAL-backed ingestion path is intentionally narrow: one-band terrain rasters, square pixels, and no reprojection or rotated rasters in the first pass.

Once a file has been loaded into a validated `TerrainRaster`, the core also
owns the contract-to-grid adaptation step so examples and future tools do not
need to duplicate that mapping logic.

## Run the example

```bash
./build/floodsim_simple_grid
```

The example runs a toy `10x10` terrain, applies rainfall for several steps, and prints water-depth totals plus the final grid.

It can also export a self-describing CSV file:

```bash
./build/floodsim_simple_grid final_grid.csv
python3 examples/simple_grid/inspect_export.py final_grid.csv
```

The companion consumer example reads the exported CSV contract and prints a compact summary of the grid metadata and simulated water results. Terrain-derived exports also include origin and CRS metadata when available from ingestion.

## Run the first real-terrain workflow

The repository now also includes a small Phase 2 example that loads a committed
GeoTIFF terrain clip through GDAL, runs rainfall on it, and exports the final
grid as CSV:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output.csv
python3 examples/simple_grid/inspect_export.py real_terrain_output.csv
```

The real-terrain example also accepts a small set of optional scenario flags
for repeatable local runs. Those values are parsed into one explicit scenario
configuration before the example constructs rainfall and simulation settings.
It also supports a few documented named rainfall presets for repeatable local
comparison:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output_heavier_rain.csv \
  --scenario intense_short
```

It now also supports a narrow external scenario-definition CSV so reviewed
scenario inputs do not need to remain compiled into the example:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_from_file.csv \
  --scenario-file examples/real_terrain/data/sample_single_scenario.csv
```

That reviewed scenario-file contract can now describe either:

- a constant-intensity event
- or a profile-backed event that references one external per-step rainfall CSV

The same example now supports a minimal batch path for running several named
scenarios over the same clip with deterministic per-scenario output paths:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_batch.csv \
  --batch-scenarios baseline,intense_short,long_moderate
```

That batch run also writes one compact comparison CSV beside the per-scenario
outputs so the first real-scenario workflow does not require manually reading
every scenario export in isolation.

Those scenario inputs now include both `runoff_coefficient` and a narrow
`initial_loss_m` control. Together they let the MVP represent:

- a retained fraction of rainfall that becomes immediate surface water
- a simple event-start loss depth that must be satisfied before ponding begins

This remains a screening-oriented approximation, not a calibrated infiltration
or drainage model.

The same workflow can now also load one small external rainfall-profile CSV so
the event shape does not need to stay constant over the whole run:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/drainage_slope.asc \
  real_terrain_profile.csv \
  --rainfall-profile-file examples/real_terrain/data/sample_storm_profile.csv
```

That contract keeps the time step fixed and supplies one uniform intensity per
simulation step. It is the first narrow path toward reviewed real storm
scenarios without adding a broader scenario-management subsystem yet.

The same workflow now also supports one narrow per-cell surface-class overlay
so selected cells can behave as impervious areas while the rest of the clip
stays pervious:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/drainage_slope.asc \
  real_terrain_surface_classes.csv \
  --runoff-coefficient 0.40 \
  --initial-loss-m 0.002 \
  --surface-class-file examples/real_terrain/data/drainage_slope_surface_classes.csv \
  --impervious-runoff-coefficient 1.0 \
  --impervious-initial-loss-m 0.0
```

That keeps the MVP raster-first: one small CSV marks impervious cells, while
the reviewed scenario or CLI still defines the shared storm itself.

The same workflow can also emit deterministic intermediate snapshot CSVs using
`--snapshot-every-steps <count>` so scenario timing can be inspected before a
richer viewer exists.
The repository now includes a lightweight local debugging viewer for exported
FloodSim CSVs, snapshot series, and terrain rasters under
`examples/real_terrain/debug_viewer.py`. GeoTIFF support in that viewer now
flows through the same GDAL-backed ingestion path used by the real-terrain
simulation workflow.

The real-terrain example also now supports one narrow area-definition CSV
contract for repeatable local DEM clip selection with provenance:

```bash
./build/floodsim_real_terrain_example \
  --area-file examples/real_terrain/data/sample_area_clip.csv \
  real_terrain_area_clip.csv
```

That contract keeps the MVP scope narrow: one DEM path, one optional pixel
window, and explicit provenance fields. More detail is in
[docs/terrain_ingestion_contract.md](/home/sergio/dev/flood-sim/docs/terrain_ingestion_contract.md:1)
and [examples/real_terrain/README.md](/home/sergio/dev/flood-sim/examples/real_terrain/README.md:1).

There is now also one staged external-source variant for free DEM workflows:

```bash
./build/floodsim_real_terrain_example \
  --external-area-file examples/real_terrain/data/sample_external_area_clip.csv \
  real_terrain_external_area_clip.csv
```

That path still expects a locally available staged DEM file, but it now
materializes that source into a deterministic cache and preserves source kind,
URL, license, and cache identity in reports and CSV metadata.

Viewer quickstart:

```bash
python3 examples/real_terrain/debug_viewer.py real_terrain_output.csv
```

If you pass one final FloodSim CSV, the viewer automatically discovers matching
snapshot CSVs beside it and lets you step through them with the left and right
arrow keys or the `Prev` and `Next` buttons. Use the viewer `Scale` control to
switch between `dynamic-per-frame` coloring and a `fixed-series` scale when you
want stable cross-frame comparison.

GeoTIFF inputs require the helper binary built by the normal CMake flow:

```bash
cmake -S . -B build
cmake --build build
python3 examples/real_terrain/debug_viewer.py \
  examples/real_terrain/data/sample_dem.tif
```

By default the viewer looks for `build/floodsim_terrain_debug_export`. You can
override that path with `--terrain-export-binary` or by setting
`FLOODSIM_TERRAIN_DEBUG_EXPORT=/path/to/floodsim_terrain_debug_export`.

You can still override preset values directly when experimenting:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output_heavier_rain.csv \
  --scenario baseline \
  --rainfall-intensity-m-per-hour 0.020 \
  --time-step-seconds 600 \
  --steps 4
```

This example is intentionally small and deterministic. It proves the first
real-format ingestion path without claiming city-scale realism yet.

For regression coverage, the repository now also carries a small curated set of
named real-terrain fixtures under `examples/real_terrain/data`: a nodata-aware
basin with CRS metadata, a monotonic drainage slope, a flat ponding case, an
edge-notch nodata/outflow case, and a slightly larger barrier-style urban-ish
case. Those fixtures are kept as example/test assets so engine configuration
stays limited to terrain, rainfall, timing, runoff, and boundary settings
rather than absorbing test-specific fixture semantics.
The real-terrain example itself is now split into a thin CLI plus a reusable
C++ helper layer so most workflow behavior can be tested without relying on a
process-level script.

The real-terrain loader also emits a compact ingestion summary so nodata
handling, valid-cell counts, and clipping loss are visible during scenario
review instead of being implicit.

Real-terrain CSV exports now also carry scenario and timing metadata in the
metadata preamble so repeated runs can be identified safely during comparison.
The example also prints a compact deterministic summary line so quick run
comparison does not require reading the full per-cell CSV first.
Real-terrain runs now default to `--boundary-mode open` so clipped terrain
edges behave more like narrow outflow boundaries than retaining walls. You can
still force `--boundary-mode closed` for conservative comparisons or toy-style
retention tests. They can also choose a simple `--runoff-coefficient` to
approximate that not all rainfall becomes retained surface water immediately.
The example documentation now includes one canonical comparison walkthrough
over the committed sample clip so another contributor can rerun the same named
scenarios and inspect the expected artifacts deterministically.

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Next steps

The intended evolution is:

1. keep the toy grid stable and well tested
2. harden one repeatable real-terrain workflow with explicit scenario and export contracts
3. add the first realism-bearing model improvements, such as better edge behavior or simple rainfall-loss controls
4. then broaden visualization, scenario comparison, and urban-surface behavior

More detail is in [docs/architecture.md](/home/sergio/dev/flood-sim/docs/architecture.md), [docs/simulation_model.md](/home/sergio/dev/flood-sim/docs/simulation_model.md), and [docs/roadmap.md](/home/sergio/dev/flood-sim/docs/roadmap.md).

## Phase 1 paper

The repository also includes a LaTeX paper for the implemented Phase 1 simulator in [docs/paper/README.md](/home/sergio/dev/flood-sim/docs/paper/README.md:1). It documents the toy-grid model, rainfall and boundary contracts, routing pseudocode, representative figures, and the main limitations of the current phase.
