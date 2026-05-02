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
- apply uniform rainfall before each flow step
- route a fraction of water to lower orthogonal neighbors using surface height (`elevation + water depth`)
- accumulate transfers across the full grid and apply them after the scan completes

Later responsibilities may include:

- import-ready raster adapters
- boundary conditions
- drainage and impervious surface effects
- better time stepping and calibration hooks

### 3. Output raster and tiles

Simulation outputs should eventually be exportable as:

- raster flood-depth layers
- tiled outputs for map display
- summary metrics for scenario comparison

The MVP currently prints CLI output only.

### 4. Graphical visualization

A future visualization layer can render flood depth over basemaps and city layers. This may begin as a lightweight local viewer and later evolve into a richer graphical application if the simulation outputs and workflows justify it. It is intentionally not included in the first version of this repository.
