# Architecture

FloodSim is planned as a pipeline with clear boundaries between data preparation, simulation, and visualization.

## Planned flow

`data ingestion -> simulation core -> output raster/tiles -> graphical visualization`

## Components

### 1. Data ingestion

The ingestion layer will prepare terrain and scenario inputs:

- DEM and GeoTIFF terrain loading
- raster normalization and clipping
- scenario metadata such as rainfall intensity and duration

The build now supports optional GDAL integration as the intended path for the first real DEM and GeoTIFF ingestion work. That support should stay narrow in the early Phase 2 implementation: one-band terrain rasters first, broader GIS workflows later.

For the first real-terrain workflow, ingestion should target the contract described in [docs/terrain_ingestion_contract.md](/home/sergio/dev/flood-sim/docs/terrain_ingestion_contract.md). That contract keeps the initial imported terrain object narrow: raster dimensions, square cell size, row-major elevations, a valid-cell mask for nodata handling, and optional origin / CRS metadata preserved for later map alignment.

Terrain clipping is now an explicit ingestion-layer capability rather than only
an example-CLI convenience. Callers can request either the full source raster
or a bounded pixel window and still receive the same validated `TerrainRaster`
contract.

When inspection or debugging matters, the same ingestion path can also return a
separate `TerrainIngestionReport` describing nodata metadata presence,
valid-domain counts, and clipping loss without changing the simulation-facing
terrain contract.

The contract-to-simulation handoff is explicit as well: validated
`TerrainRaster` data is adapted into `Grid` through a dedicated core helper so
examples and future ingestion tools do not each reimplement that mapping.
The real-terrain example now treats rainfall and timing inputs similarly by
building one narrow scenario configuration object before constructing the
simulation-facing rainfall and step settings.

### 2. Simulation core

The simulation core is implemented in C++20 for deterministic behavior and future performance headroom.

Current responsibilities:

- store terrain elevation and water depth on a raster grid
- apply uniform rainfall before each flow step, with rainfall scenario intensity expressed in meters per hour and converted using `time_step_seconds`
- route a fraction of water to lower orthogonal neighbors using surface height (`elevation + water depth`)
- accumulate transfers across the full grid and apply them after the scan completes
- expose boundary handling as an explicit simulation setting, with Phase 1 currently supporting closed boundaries only
- treat invalid imported-terrain cells as out-of-domain cells that do not receive rainfall and cannot receive routed flow

Later responsibilities may include:

- import-ready raster adapters
- additional boundary modes such as open edge outflow
- drainage and impervious surface effects
- better time stepping and calibration hooks

### 3. Output raster and tiles

Simulation outputs should eventually be exportable as:

- raster flood-depth layers
- tiled outputs for map display
- summary metrics for scenario comparison

The Phase 1 MVP now supports a minimal CSV export path for toy-grid runs, and the same contract is reused for the first real-terrain example. The current export format begins with metadata lines for:

- `floodsim_csv_version`
- `rows`
- `cols`
- `cell_size_m`

Terrain-derived exports may also include:

- `scenario_name`
- `rainfall_intensity_m_per_hour`
- `time_step_seconds`
- `total_duration_seconds`
- `origin_x_m`
- `origin_y_m`
- `crs_id`

After the metadata preamble, the file writes one row per cell with:

- `row`
- `col`
- `elevation_m`
- `water_depth_m`
- `surface_height_m`

This keeps the output stable and self-describing for scripts or a future viewer without introducing heavier raster or GIS dependencies yet. The optional scenario and timing fields let real-terrain runs carry enough run identity for safe comparison, while the optional origin / CRS fields preserve map placement metadata without changing the per-cell table layout.

The first real-terrain example workflow now lives in
[examples/real_terrain/README.md](/home/sergio/dev/flood-sim/examples/real_terrain/README.md:1).
It exercises the narrow GDAL ingestion path, imported-domain handling, the
existing simulation step, and CSV export together on a committed sample
GeoTIFF.

### 4. Graphical visualization

A future visualization layer can render flood depth over basemaps and city layers. This may begin as a lightweight local viewer and later evolve into a richer graphical application if the simulation outputs and workflows justify it. It is intentionally not included in the first version of this repository.

## Technical paper artifact

The repository also carries a LaTeX paper describing the implemented Phase 1 simulator in `docs/paper/phase1_simulator.tex`.

That paper is intended to:

- describe the current toy-grid model as implemented
- capture rainfall, routing, boundary, and export assumptions in one place
- provide a compact technical artifact for review, presentation, and later extension

It should stay aligned with the actual code and tests rather than getting ahead of the implementation roadmap.
