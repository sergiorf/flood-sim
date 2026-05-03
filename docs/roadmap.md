# Roadmap

FloodSim is intended to become a real product for users such as city planners, infrastructure teams, risk analysts, and insurers.

That means the roadmap should not be read only as an engineering sequence. It should be read as a capability ladder from a trustworthy toy kernel to a usable urban flood-risk workflow.

## Product goal

The long-term product goal is to support workflows such as:

- screening flood exposure for neighborhoods, roads, parcels, and critical assets
- comparing drainage, land-use, and infrastructure scenarios
- exploring rainfall-event and climate-risk scenarios over real city terrain
- exporting map-aligned depth outputs and summary metrics for downstream reporting
- producing evidence that planning, resilience, and insurance teams can review and communicate

This repository is not at that product state yet. The early phases focus on building a simulation core that is explicit, testable, and extensible enough to grow into that product without unstable semantics or ad hoc formats.

## Current planning focus

Phase 1 is complete.

Active planning should now target Phase 2: DEM import and the first real-terrain workflow.

## Phase 1. Toy grid simulation

Build a clear, tested raster-grid prototype with rainfall accumulation and simple downhill flow.

Capabilities gained:

- deterministic simulation behavior with explicit rainfall, routing, and boundary semantics
- stable exported outputs that downstream tools can consume
- a small runnable example and tests that keep the toy-grid contract from drifting

Why this phase matters:

- it proves software shape and simulation contracts before real-data complexity is introduced
- it reduces the risk of building ingestion, viewers, or scenarios on top of unstable behavior

Phase 1 is complete when the repository has:

- a buildable C++20 simulation core
- documented and tested toy-grid semantics
- a simple example workflow
- a stable export contract suitable for later tooling

Phase 1 is not yet a sellable urban flood product. It is the kernel that later product capabilities will depend on.

## Phase 2. DEM import

Add terrain ingestion from DEM and GeoTIFF sources, likely with optional GDAL integration.

Capabilities gained:

- simulation over real terrain rather than hand-built toy grids
- explicit handling of raster extent, resolution, and nodata behavior
- the first end-to-end workflow on a clipped real-world area

Why this phase matters:

- this is the first phase where FloodSim starts becoming geographically meaningful
- it creates the bridge from toy prototype to a credible real-terrain workflow

Expected outcome:

- a user can load a small real terrain clip, run a rainfall case, and export map-aligned outputs for inspection
- early Phase 2 work should start from a narrow validated terrain contract before adding broader DEM or GeoTIFF support

## Phase 3. Map visualization

Expose outputs in forms that can be rendered over real maps and tiles.

Capabilities gained:

- flood-depth outputs that can be overlaid on real basemaps
- easier inspection of where water accumulates relative to streets, parcels, and terrain features
- a clearer path to analyst and planner workflows

Why this phase matters:

- flood outputs become much more interpretable once they can be compared to real geography
- this is where technical outputs start becoming usable planning artifacts rather than internal model artifacts

## Phase 4. Graphical viewer

Add a lightweight graphical app for inspecting terrain, water depth, and scenario outputs without changing the MVP focus on a simple simulation core. Start with visualization of exported outputs before considering richer interactive tooling.

Capabilities gained:

- local visual inspection of runs without relying on raw CSV or ad hoc scripts
- faster scenario review for engineering, planning, and product iteration
- a first user-facing interface layer on top of the simulation outputs

Why this phase matters:

- a viewer improves iteration speed and user trust
- it becomes easier to demonstrate the product to non-developer stakeholders

## Phase 5. Rainfall scenarios

Support configurable rainfall events, durations, intensities, and scenario comparison.

Capabilities gained:

- richer event design than a single uniform rainfall setting
- comparative scenario workflows such as baseline vs. intense storm or short burst vs. long event
- better alignment with planning and risk-analysis questions

Why this phase matters:

- planners and insurers care about scenario differences, not only single-run outputs
- this is where the simulator starts answering decision questions rather than only producing physics-like fields

## Phase 6. Urban drainage and buildings

Incorporate simplified urban drainage effects, impervious surfaces, and obstacle/building representations.

Capabilities gained:

- basic representation of urban form rather than terrain alone
- more realistic runoff and flow concentration behavior in built environments
- stronger relevance for city-scale planning, infrastructure screening, and asset risk workflows

Why this phase matters:

- real urban flood behavior depends heavily on drainage, imperviousness, and built obstacles
- this is a major step toward a product that can support practical city and insurance use cases

## Phase 7. Climate-risk scenarios

Model future rainfall and climate-change risk scenarios for city-scale planning workflows.

Capabilities gained:

- forward-looking scenario analysis rather than only present-day event simulation
- comparison of infrastructure and planning choices under changing hazard conditions
- stronger relevance for resilience planning, long-horizon capital allocation, and insurance risk framing

Why this phase matters:

- this is the phase where FloodSim becomes aligned with climate adaptation and long-term risk planning workflows

## Bridge To Product

The critical bridge from prototype to credible product is:

1. stable simulation semantics
2. real-terrain ingestion
3. map-aligned outputs
4. scenario comparison
5. urban surface realism

Until Phase 2 and Phase 3 are in place, the repository is still mainly proving the simulation kernel.

Once Phase 2 through Phase 6 are substantially in place, FloodSim can begin to support real pilot workflows for:

- municipal screening and resilience planning
- infrastructure exposure review
- parcel or asset risk exploration
- insurance and underwriting support for flood-prone urban areas

## First Real-Terrain Milestone

Before claiming practical usefulness on real areas, FloodSim should be able to:

- load a clipped DEM or GeoTIFF terrain raster
- define raster resolution and nodata behavior explicitly
- run rainfall over that real terrain clip
- export results in a map-aligned format
- inspect the output visually against real geography

That milestone is the first point where the product path becomes concrete rather than aspirational.

The repository now includes a first committed real-terrain example using a tiny
GeoTIFF clip. The remaining Phase 2 work is about broadening and hardening that
path rather than proving it from scratch.

## Working style

Use this roadmap for phase-level direction, not as a task tracker. Convert the active phase into a short iteration plan and create tickets only for changes large enough to need discussion, acceptance criteria, or explicit follow-up.

See [docs/planning_workflow.md](/home/sergio/dev/flood-sim/docs/planning_workflow.md) for the proposed roadmap-to-ticket workflow.
