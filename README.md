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

The companion consumer example reads the exported Phase 1 CSV contract and prints a compact summary of the grid metadata and simulated water results.

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

This example is intentionally small and deterministic. It proves the first
real-format ingestion path without claiming city-scale realism yet.

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Next steps

The intended evolution is:

1. keep the toy grid stable and well tested
2. add DEM import, likely through GDAL integration
3. produce exportable raster outputs for visualization
4. layer in richer rainfall and urban-surface behavior

More detail is in [docs/architecture.md](/home/sergio/dev/flood-sim/docs/architecture.md), [docs/simulation_model.md](/home/sergio/dev/flood-sim/docs/simulation_model.md), and [docs/roadmap.md](/home/sergio/dev/flood-sim/docs/roadmap.md).

## Phase 1 paper

The repository also includes a LaTeX paper for the implemented Phase 1 simulator in [docs/paper/README.md](/home/sergio/dev/flood-sim/docs/paper/README.md:1). It documents the toy-grid model, rainfall and boundary contracts, routing pseudocode, representative figures, and the main limitations of the current phase.
