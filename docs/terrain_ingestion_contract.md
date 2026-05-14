# Terrain Ingestion Contract

This document defines the Phase 2 terrain-ingestion contract for FloodSim.

The purpose of the contract is to separate:

- how terrain data is read from files such as DEMs or GeoTIFFs
- what validated raster object the simulation-adjacent code may rely on

For the first real-terrain workflow, every ingestion path should produce a `TerrainRaster`-style object with the fields documented below.

Once validated, that terrain contract is converted into the simulation `Grid`
through a dedicated core helper. This keeps file-format concerns in ingestion
code and keeps example binaries thin.

## Plain-language glossary

These terms appear frequently in Phase 2 work. For the first implementation, a practical understanding is enough.

### DEM

DEM stands for Digital Elevation Model.

For FloodSim, think of it as:

- a height map of the ground
- one elevation value per raster cell

The simulator does not need a perfect GIS worldview first. It needs a real terrain grid.

### GeoTIFF

A GeoTIFF is a TIFF raster file with geospatial metadata attached.

For FloodSim, that usually means a file can carry:

- raster dimensions
- cell size
- georeferencing
- CRS metadata
- nodata metadata
- elevation values

Many DEM datasets are distributed as GeoTIFF files.

### Raster

A raster is a rectangular grid of cells.

For this project, a raster means:

- rows and columns
- one value stored per cell

In Phase 2, the raster values of interest are terrain elevations.

### Single-band raster

A band is one layer of values in a raster file.

A single-band raster means:

- each cell stores one main value
- for DEM work, that value is terrain elevation

This is a useful first limitation because the first loader only needs one terrain layer, not a multi-layer remote-sensing stack.

### CRS

CRS stands for Coordinate Reference System.

For practical Phase 2 work, it answers:

- what map coordinate system the raster uses
- how the terrain should line up with other spatial data later

The first simulation step does not compute differently because of CRS, but later export and mapping work will care about it.

### Nodata

Nodata means a raster cell does not contain valid terrain data.

Common reasons:

- clipped edges
- gaps in the source data
- masked-out areas

For the first FloodSim terrain contract:

- nodata cells stay present in the raster shape
- nodata cells are excluded from the simulation domain using `valid_cell_mask`

### Reprojection

Reprojection means converting spatial data from one CRS into another.

Example:

- source data may arrive in one map coordinate system
- later visualization or comparison layers may use another

Phase 2 does not need full reprojection support yet. The first loader can preserve the source CRS and fail on unsupported cases rather than transforming everything automatically.

## Contract summary

Required fields for Phase 2 simulation use:

- `rows`
- `cols`
- `cell_size_m`
- `elevation_m`
- `valid_cell_mask`

Optional fields preserved for later map alignment and export:

- `origin_x_m`
- `origin_y_m`
- `crs_id`

Related ingestion-only reporting for inspection and debugging:

- `TerrainIngestionReport`
- source raster dimensions
- loaded window dimensions
- valid and invalid cell counts
- number of cells excluded by clipping
- explicit nodata status, including whether band metadata was present

The Phase 2 GDAL path may populate that contract from either:

- a full source raster
- a clipped rectangular pixel window inside a source raster

When a window is used, the contract still preserves the clipped raster shape
exactly, keeps row-major indexing local to the clipped result, and shifts the
optional origin metadata to the clipped top-left cell.

## Field definitions

### `rows`

Meaning:
- number of raster rows

Type:
- positive integer

Why it exists:
- the simulation core needs explicit raster dimensions
- imported terrain must preserve source shape exactly

Rule:
- must be greater than zero

Example:
- `rows = 2`

### `cols`

Meaning:
- number of raster columns

Type:
- positive integer

Why it exists:
- together with `rows`, defines the raster extent in cells

Rule:
- must be greater than zero

Example:
- `cols = 3`

### `cell_size_m`

Meaning:
- ground size of one raster cell edge, in meters

Type:
- positive floating-point value

Why it exists:
- real terrain is not just an abstract matrix
- cell size will matter for interpreting imported terrain and later map-aligned outputs

Phase 2 simplification:
- the first contract assumes square cells
- anisotropic pixel sizes are out of scope for the first ingestion milestone

Rule:
- must be greater than zero

Example:
- `cell_size_m = 2.0`

### `elevation_m`

Meaning:
- terrain elevation values in meters

Type:
- row-major array of floating-point values

Why it exists:
- this is the core DEM payload the simulator will eventually use to populate terrain elevations

Indexing rule:
- array length must equal `rows * cols`
- row-major means `(row, col)` maps to `row * cols + col`

Important Phase 2 rule:
- `elevation_m` keeps one entry for every raster cell, even if a cell is invalid
- nodata or out-of-domain cells are filtered by `valid_cell_mask`, not by shrinking the array

Example:
- for a `2 x 3` raster:

```text
rows = 2
cols = 3
elevation_m = [
  101.2, 100.7, 100.1,
   99.9,  99.4,  98.8
]
```

### `valid_cell_mask`

Meaning:
- row-major mask describing which raster cells belong to the simulation domain

Type:
- row-major array aligned with `elevation_m`
- `1` means valid simulation cell
- `0` means nodata or out-of-domain cell

Why it exists:
- real terrain files often include nodata cells
- the simulation needs explicit domain membership instead of silently inventing elevations

Phase 2 default behavior:
- valid cells form the simulation domain
- nodata cells are excluded from the domain
- later simulation work should not route flow into nodata cells
- later simulation work should not apply rainfall to nodata cells

Rules:
- array length must equal `rows * cols`
- the raster must contain at least one valid cell

Example:

```text
valid_cell_mask = [
  1, 1, 1,
  1, 0, 1
]
```

In that example, the middle cell of the second row is present in the raster but excluded from the simulation domain.

### `origin_x_m`

Meaning:
- optional real-world x coordinate of the raster origin

Type:
- optional floating-point value

Why it exists:
- later export and visualization work will need to place the raster back into geographic space

Phase 2 rule:
- optional for the first simulation-adjacent contract
- if origin metadata is present, both `origin_x_m` and `origin_y_m` must be present

Example:
- `origin_x_m = 154320.0`

### `origin_y_m`

Meaning:
- optional real-world y coordinate of the raster origin

Type:
- optional floating-point value

Why it exists:
- pairs with `origin_x_m` to preserve map placement for later phases

Phase 2 rule:
- optional for the first ingestion contract
- must appear together with `origin_x_m`

Example:
- `origin_y_m = 171205.0`

### `crs_id`

Meaning:
- optional identifier for the coordinate reference system

Type:
- optional string

Why it exists:
- exported results will eventually need enough metadata to align with maps, assets, and planning layers

Phase 2 rule:
- preserved when available
- not yet required for the first simulation step logic

Examples:
- `crs_id = "EPSG:31370"`
- `crs_id = "EPSG:3857"`

## Full example

The following example describes a small imported terrain raster:

```text
rows = 2
cols = 3
cell_size_m = 2.0

elevation_m = [
  101.2, 100.7, 100.1,
   99.9,   0.0,  98.8
]

valid_cell_mask = [
  1, 1, 1,
  1, 0, 1
]

origin_x_m = 154320.0
origin_y_m = 171205.0
crs_id = "EPSG:31370"
```

Interpretation:

- the raster is `2 x 3`
- each cell represents a `2m x 2m` square on the ground
- one cell is invalid because the mask contains `0`
- that invalid cell still has an array slot in `elevation_m`, but it is excluded from the simulation domain
- the origin and CRS are preserved for later map-aligned workflows

## Validation rules

Any first-pass terrain-ingestion path should validate the contract before handing terrain to later steps.

Minimum validation:

- `rows > 0`
- `cols > 0`
- `cell_size_m > 0`
- `elevation_m.size() == rows * cols`
- `valid_cell_mask.size() == rows * cols`
- at least one valid cell exists
- `origin_x_m` and `origin_y_m` are both present or both absent

## First GDAL-backed loader scope

The first implementation of Phase 2 ingestion is intentionally narrow.

Supported in the first pass:

- one raster band
- one elevation value per cell
- square pixels
- no raster rotation or shear
- optional nodata value mapped into `valid_cell_mask`
- preserved origin and CRS metadata when available

Runtime behavior coupled to that loader:

- invalid cells behave as out-of-domain cells
- rainfall is applied only to valid cells
- routing ignores invalid neighbors entirely
- clipped or nodata-adjacent gaps do not create extra receiver cells by themselves
- open-boundary outflow applies only across missing orthogonal neighbors at the raster edge, not across internal nodata gaps

The loader now also exposes a narrow `TerrainIngestionReport` alongside the
validated `TerrainRaster` when callers need inspectable ingestion metadata.

That report is intentionally separate from the simulation-facing terrain
contract. Its purpose is to explain what the loader saw and how it interpreted
nodata and clipping, not to change the data the simulation consumes.

Current report fields:

- source raster `rows` and `cols`
- loaded raster `rows` and `cols`
- `valid_cell_count`
- `invalid_cell_count`
- `clipped_cell_count`
- `window_applied`
- `nodata_metadata_present`
- `nan_cell_count`
- `nodata_status`

Current `nodata_status` values:

- `BandMetadataApplied`: the raster band exposed nodata metadata and the loader mapped matching cells into `valid_cell_mask`
- `BandMetadataMissingAllCellsValid`: no nodata metadata was present and no `NaN` cells were seen in the loaded raster
- `BandMetadataMissingNaNCellsPresent`: no nodata metadata was present, but at least one loaded cell contained `NaN`, so the loader reports that condition explicitly for review

This reporting keeps missing or suspicious nodata metadata visible in example
logs and tests instead of relying on implicit assumptions.

Rejected in the first pass:

- multi-band rasters
- rotated or sheared rasters
- non-square pixels
- files that do not expose the minimum geotransform metadata needed to derive cell size

Why keep it narrow:

- it anchors the project on real DEM and GeoTIFF formats immediately
- it avoids pretending the first loader is already a full GIS pipeline
- it keeps errors explicit while the first real-terrain workflow is still being established

## What this contract does not solve yet

This contract is intentionally narrow. It does not yet define:

- how DEM or GeoTIFF files are parsed
- multi-band raster handling
- reprojection workflows
- anisotropic pixels
- vertical datum management
- exact simulation behavior at nodata boundaries

Those belong to later Phase 2 tickets. This contract only defines the validated terrain object they should target.
