# Simple Grid Example

This example runs a toy `10x10` bowl-shaped terrain with uniform rainfall and a simple downhill redistribution rule.

The current Phase 1 model exposes boundary handling through the simulation configuration and currently supports the `Closed` mode only. Water can move only between cells inside the grid and cannot flow out across the outer edge of the raster.

Build and run from the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/floodsim_simple_grid
```

To also export the final grid state as CSV:

```bash
./build/floodsim_simple_grid final_grid.csv
```

The CSV contains one row per cell with this schema:

```text
row,col,elevation_m,water_depth_m,surface_height_m
```

Rows are written in row-major order. This keeps the Phase 1 output format simple for scripts and future viewers while preserving both terrain and simulated water depth in a single file.
