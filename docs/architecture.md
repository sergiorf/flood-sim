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

The initial repository does not require GDAL yet, but the code structure is intended to make that integration straightforward later.

### 2. Simulation core

The simulation core is implemented in C++20 for deterministic behavior and future performance headroom.

Current responsibilities:

- store terrain elevation and water depth on a raster grid
- apply uniform rainfall before each flow step, with rainfall scenario intensity expressed in meters per hour and converted using `time_step_seconds`
- route a fraction of water to lower orthogonal neighbors using surface height (`elevation + water depth`)
- accumulate transfers across the full grid and apply them after the scan completes
- expose boundary handling as an explicit simulation setting, with Phase 1 currently supporting closed boundaries only

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

The Phase 1 MVP now supports a minimal CSV export path for toy-grid runs. The current export format begins with metadata lines for:

- `floodsim_csv_version`
- `rows`
- `cols`
- `cell_size_m`

After the metadata preamble, the file writes one row per cell with:

- `row`
- `col`
- `elevation_m`
- `water_depth_m`
- `surface_height_m`

This keeps the output stable and self-describing for scripts or a future viewer without introducing heavier raster or GIS dependencies yet.

### 4. Graphical visualization

A future visualization layer can render flood depth over basemaps and city layers. This may begin as a lightweight local viewer and later evolve into a richer graphical application if the simulation outputs and workflows justify it. It is intentionally not included in the first version of this repository.

## Technical paper artifact

The repository also carries a LaTeX paper describing the implemented Phase 1 simulator in `docs/paper/phase1_simulator.tex`.

That paper is intended to:

- describe the current toy-grid model as implemented
- capture rainfall, routing, boundary, and export assumptions in one place
- provide a compact technical artifact for review, presentation, and later extension

It should stay aligned with the actual code and tests rather than getting ahead of the implementation roadmap.
