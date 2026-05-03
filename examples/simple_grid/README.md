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

The CSV starts with a small metadata preamble and then one row per cell:

```text
# floodsim_csv_version,1
# rows,<grid_rows>
# cols,<grid_cols>
# cell_size_m,<cell_size_m>
row,col,elevation_m,water_depth_m,surface_height_m
```

The metadata lines make the export self-describing for downstream tools. Data rows are written in row-major order. Lines beginning with `# ` are metadata; the first non-metadata line is the CSV column header.
