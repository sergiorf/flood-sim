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
- rainfall is applied before flow during each step
- only the 4 orthogonal neighbors participate in flow routing
- routing compares full water surface height, not terrain elevation alone
- only neighbors with strictly lower surface height receive flow
- outflow is capped as a fraction of the source cell's water depth for that step
- transfers are accumulated and applied after the grid scan completes
- cells with flat or higher neighboring surfaces do not shed water during that step

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
- configurable or validated boundary conditions

Any outputs from this version should be treated as prototype behavior only.
