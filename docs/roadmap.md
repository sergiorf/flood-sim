# Roadmap

## Phase 1. Toy grid simulation

Build a clear, tested raster-grid prototype with rainfall accumulation and simple downhill flow.

## Phase 2. DEM import

Add terrain ingestion from DEM and GeoTIFF sources, likely with optional GDAL integration.

## Phase 3. Map visualization

Expose outputs in forms that can be rendered over real maps and tiles.

## Phase 4. Graphical viewer

Add a lightweight graphical app for inspecting terrain, water depth, and scenario outputs without changing the MVP focus on a simple simulation core. Start with visualization of exported outputs before considering richer interactive tooling.

## Phase 5. Rainfall scenarios

Support configurable rainfall events, durations, intensities, and scenario comparison.

## Phase 6. Urban drainage and buildings

Incorporate simplified urban drainage effects, impervious surfaces, and obstacle/building representations.

## Phase 7. Climate-risk scenarios

Model future rainfall and climate-change risk scenarios for city-scale planning workflows.

## Working style

Use this roadmap for phase-level direction, not as a task tracker. Convert the active phase into a short iteration plan and create tickets only for changes large enough to need discussion, acceptance criteria, or explicit follow-up.

See [docs/planning_workflow.md](/home/sergio/dev/flood-sim/docs/planning_workflow.md) for the proposed roadmap-to-ticket workflow.
