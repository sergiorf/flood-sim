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

The current algorithm is deliberately simple:

1. add rainfall depth to every cell
2. inspect each cell's local surface height
3. move a fraction of water to lower neighboring cells
4. apply transfers after scanning the full grid

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
- validated boundary conditions

Any outputs from this version should be treated as prototype behavior only.

