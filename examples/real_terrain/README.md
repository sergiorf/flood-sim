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

The example uses a fixed configuration so the workflow is reproducible:

- rainfall intensity: `0.012 m/hour`
- step duration: `300 seconds`
- number of steps: `12`
- boundary mode: `Closed`

The example prints a short load and simulation summary, then writes the same
CSV contract used elsewhere in the repository with added georeferencing
metadata from the source raster:

```text
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
- imported nodata is preserved as out-of-domain cells
- the simulation can run on the imported terrain
- the final state can be exported with origin / CRS metadata for later inspection and visualization work

What it does not prove yet:

- map rendering
- reprojection workflows
- large-area performance
- urban drainage, buildings, or calibration
