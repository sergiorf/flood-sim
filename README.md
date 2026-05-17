# FloodSim

FloodSim is an urban flood simulation project focused on rainfall-driven
surface flooding over real city terrain.

The current product posture is deliberately narrow:

- a C++20 raster-grid simulation core
- simple, testable rainfall and overland-flow behavior
- a small real-terrain workflow for local demos and iteration

It is not a certified hydrology or hydrodynamics engine. The goal of the
current MVP is clarity, buildability, and a clean base for richer modeling
later.

## Repository layout

```text
flood-sim/
  apps/               local product-facing applications
  core/               C++ simulation library and tests
  data/scripts/       preprocessing and validation helpers
  docs/               architecture, model notes, roadmap, contracts
  examples/           runnable examples and workflow helpers
  terrain_library/    curated terrain sources and area notes
```

## Build

Requirements:

- CMake 3.20+
- a C++20 compiler
- `doctest` if you want to build the C++ test suite
- GDAL if you want the real-terrain ingestion path

```bash
cmake -S . -B build
cmake --build build
```

Optional configuration:

```bash
cmake -S . -B build -DFLOODSIM_BUILD_TESTS=OFF
cmake -S . -B build -DFLOODSIM_ENABLE_GDAL=OFF
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Run

Toy grid example:

```bash
./build/floodsim_simple_grid
```

Real-terrain example:

```bash
./build/floodsim_real_terrain_example \
  examples/real_terrain/data/sample_dem.tif \
  real_terrain_output.csv
```

Local CSV and terrain inspection viewer:

```bash
./build/apps/native_viewer/floodsim_native_viewer real_terrain_output.csv
```

## Where the rest lives

Detailed workflow, scenario, and export usage has been moved out of this file:

- [docs/architecture.md](/home/sergio/dev/flood-sim/docs/architecture.md:1)
- [docs/simulation_model.md](/home/sergio/dev/flood-sim/docs/simulation_model.md:1)
- [docs/roadmap.md](/home/sergio/dev/flood-sim/docs/roadmap.md:1)
- [docs/terrain_ingestion_contract.md](/home/sergio/dev/flood-sim/docs/terrain_ingestion_contract.md:1)
- [docs/terrain_library.md](/home/sergio/dev/flood-sim/docs/terrain_library.md:1)
- [examples/real_terrain/README.md](/home/sergio/dev/flood-sim/examples/real_terrain/README.md:1)
- [terrain_library/README.md](/home/sergio/dev/flood-sim/terrain_library/README.md:1)

## Terrain library

The repository includes a curated terrain-library structure for demo areas and
reviewed terrain sources. Start with:

- [docs/terrain_library.md](/home/sergio/dev/flood-sim/docs/terrain_library.md:1)
- [terrain_library/catalog.csv](/home/sergio/dev/flood-sim/terrain_library/catalog.csv:1)
- [terrain_library/areas/brussels_demo_center/README.md](/home/sergio/dev/flood-sim/terrain_library/areas/brussels_demo_center/README.md:1)

Validation helper:

```bash
python3 data/scripts/validate_terrain_library.py
```
