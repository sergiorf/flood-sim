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

## Phase 1 semantic choices

The current Phase 1 model makes these explicit choices:

- rainfall is spatially uniform across all cells
- rainfall input is expressed as intensity in meters per hour, then converted to per-step depth using `time_step_seconds`
- rainfall is applied before flow during each step
- only the 4 orthogonal neighbors participate in flow routing
- routing compares full water surface height, not terrain elevation alone
- only neighbors with strictly lower surface height receive flow
- the grid uses a closed boundary: water can move only to in-domain neighbors and cannot leave the raster across an edge
- outflow is capped as a fraction of the source cell's water depth for that step
- transfers are accumulated and applied after the grid scan completes
- cells with flat or higher neighboring surfaces do not shed water during that step

## Boundary behavior

Phase 1 uses a closed boundary condition.

In practice this means:

- cells on the edge of the grid only consider neighbors that exist inside the grid
- cells in corners have at most two orthogonal neighbors
- water does not flow off the raster, even if the terrain would appear to slope outward beyond the simulated domain
- total water in the grid changes only through rainfall, not through edge outflow

This is a deliberate simplification for the toy-grid MVP. It is less realistic than open outflow for many real landscapes, but it keeps the early model easier to reason about, easier to test, and easier to compare while core routing semantics are still being stabilized.

## Why start here

This first model is useful because it:

- is easy to reason about
- is straightforward to test
- provides a stable interface for later DEM ingestion and visualization work

## Known limitations

The current model does not yet include:

- physically rigorous shallow-water equations
- infiltration or evaporation
- drainage networks
- buildings, culverts, or sewer behavior
- calibration against observed flood events
- configurable boundary conditions
- validated edge behavior for real landscapes

Any outputs from this version should be treated as prototype behavior only.
