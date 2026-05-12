# Simulation Model

## Status

This repository starts with a simplified first simulation model. It is intended for product and engineering iteration, not for certified flood-risk assessment.

This is not a certified hydrology engine, hydraulic solver, or regulatory model.

## Current model

The MVP uses a raster/grid representation with:

- static terrain elevation per cell
- water depth per cell
- uniform rainfall added each time step
- simple downhill redistribution to lower orthogonal neighbors

## Rainfall contract

Phase 1 rainfall input is defined as a uniform intensity in meters per hour.

This is an intensity contract, not a per-step depth contract. For a step of
duration `time_step_seconds`, each cell receives:

`intensity_m_per_hour * (time_step_seconds / 3600.0)`

Implications:

- changing `time_step_seconds` changes the rainfall depth injected during each step
- multiple shorter steps accumulate the same rainfall depth as one longer step over the same total simulated duration
- rainfall is applied to every cell before any flow routing happens in that step
- a zero rainfall intensity adds no water regardless of step size

The current algorithm is deliberately simple and should be read as an exact behavioral contract for the toy-grid MVP:

1. add rainfall depth to every cell
2. inspect each cell's local surface height, defined as `terrain elevation + current water depth`
3. for each cell, identify only the orthogonal neighbors whose surface height is lower
4. move up to `max_outflow_fraction` of that cell's current water depth
5. split that outflow across lower neighbors in proportion to relative drop
6. apply all transfers after scanning the full grid, so one cell cannot relay newly received water until the next step

This means the step uses a shared snapshot of the grid state after rainfall has been added but before any per-cell flow transfers are applied.

The current model can also scale that rainfall input through a simple
`runoff_coefficient` in the simulation configuration:

`retained_rainfall_depth = intensity_m_per_hour * (time_step_seconds / 3600.0) * runoff_coefficient`

Implications:

- `runoff_coefficient = 1.0` means all rainfall becomes immediate surface water
- lower values approximate simple losses before water appears in the surface raster
- this is a practical screening control, not a calibrated infiltration model

## Phase 1 semantic choices

The current Phase 1 model makes these explicit choices:

- rainfall is spatially uniform across all cells
- rainfall input is expressed as intensity in meters per hour, then converted to per-step depth using `time_step_seconds`
- a simple `runoff_coefficient` can reduce how much rainfall becomes immediate surface water
- rainfall is applied before flow during each step
- only the 4 orthogonal neighbors participate in flow routing
- routing compares full water surface height, not terrain elevation alone
- only neighbors with strictly lower surface height receive flow
- boundary handling is an explicit simulation setting, with `Closed` and a narrow `Open` edge-outflow mode
- outflow is capped as a fraction of the source cell's water depth for that step
- transfers are accumulated and applied after the grid scan completes
- cells with flat or higher neighboring surfaces do not shed water during that step

## Boundary behavior

Phase 1 exposes boundary behavior explicitly through the simulation configuration.
At this stage, the supported modes are `Closed` and `Open`.

In practice this means:

- cells on the edge of the grid only consider neighbors that exist inside the grid
- cells in corners have at most two orthogonal neighbors
- in `Closed` mode, water does not flow off the raster, even if the terrain would appear to slope outward beyond the simulated domain
- in `Open` mode, edge cells may discharge part of their water out of the raster across missing orthogonal neighbors
- total water in the grid changes only through rainfall in `Closed` mode, but may also decrease through edge outflow in `Open` mode

`Open` mode is intentionally narrow. It is meant as a practical approximation
for clipped real-terrain runs where the raster edge should not behave like a
retaining wall. It is still much simpler than a drainage-network or calibrated
boundary treatment.

## Imported terrain domain behavior

Phase 2 imported terrain adds an explicit valid-cell domain on top of the raster shape.

For the first real-terrain workflow:

- valid cells define the simulation domain
- invalid or nodata cells remain part of the raster shape for indexing purposes
- rainfall is applied only to valid cells
- flow cannot route into invalid or nodata cells
- invalid neighbors are ignored during routing, which means clipped or nodata-adjacent edges behave like absent neighbors in the current closed-boundary model

This keeps imported terrain behavior aligned with the explicit boundary
setting. In the current real-terrain workflow, `Open` is the practical default
because clipped DEM edges usually should not behave like retaining walls, but
`Closed` remains available for conservative comparisons and regression tests.

## Why start here

This first model is useful because it:

- is easy to reason about
- is straightforward to test
- provides a stable interface for later DEM ingestion and visualization work

## Known limitations

The current model does not yet include:

- physically rigorous shallow-water equations
- calibrated infiltration or evaporation
- drainage networks
- buildings, culverts, or sewer behavior
- calibration against observed flood events
- validated edge behavior for real landscapes

Any outputs from this version should be treated as prototype behavior only.
