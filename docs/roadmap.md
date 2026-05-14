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

That work should stay disciplined. The purpose of Phase 2 is to harden one
repeatable real-terrain run path, not to spend an open-ended period expanding
toy scenario mechanics. Once the repository has enough scaffolding to run,
label, and compare a small number of deterministic real-terrain cases, priority
should shift to the first realism-bearing simulation improvements.

## Product MVP Task List

This is the current product-focused task stack for a free-data MVP aimed at
city-planning screening workflows in markets such as Europe and Brazil.

### Hydrology engine

1. Add intermediate runoff snapshots so short and long events can be compared through time, not only at final state.
2. Add a few more curated real-terrain regression clips that represent distinct planning cases such as steep drainage, flat ponding, and nodata-heavy edges.
3. Add the next realism-bearing hydrology improvement after snapshots, biased toward model meaning rather than more workflow mechanics.
4. Add benchmark-style fixture comparisons so changes in retained water, peak depth, and timing remain explainable.

### Map loading

1. Define one narrow MVP area-loading contract around a DEM clip, optional boundary, and explicit provenance.
2. Add support for clipping user-selected areas from free baseline DEM sources, starting from globally or regionally available products.
3. Normalize CRS, nodata, and clip-status reporting so imported areas are trustworthy enough for planner review.
4. Add local cache and provenance metadata so repeated area loads are reproducible and inspectable.

### Visualization

1. Add snapshot-aware exports that can support time-based review before any richer viewer exists.
2. Build a minimal local viewer that can show terrain plus flood-depth overlays over time.
   Current v1 scope: local debugging viewer for FloodSim CSV exports, snapshot series, and direct GeoTIFF viewing through the same GDAL-backed terrain ingestion path used by the simulation workflow, without a server or web stack.
3. Add scenario comparison views, including side-by-side or delta inspection.
4. Add exportable planner-facing artifacts such as screenshots, summary tables, and run provenance bundles.

### Product guardrails

1. Keep the MVP positioned as screening and scenario-comparison support, not regulatory or drainage-design analysis.
2. Prefer a few reliable free-data workflows over broad source coverage without clear interpretation.
3. Stop adding workflow mechanics once they no longer improve trust, usability, or decision value.

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

Phase 2 should end once the repository can support one credible MVP workflow:

- load a real terrain clip reproducibly
- define one explicit scenario contract
- run a small set of documented repeatable scenarios
- export outputs with enough metadata to avoid confusion
- compare runs with a small deterministic summary

The repository now has that minimum comparison path in documented form through
the committed real-terrain example and its canonical scenario walkthrough. The
remaining value in this area should come from targeted hardening or clearer
interpretation, not from turning the example into a broad orchestration layer.

Phase 2 is not meant to grow into a large scenario-management layer. If new
work mostly adds orchestration, presets, file formats, or batch mechanics
without improving interpretation or model meaning, that is a sign the phase is
drifting.

## Phase 3. Real-scenario MVP hardening

Keep only the minimum workflow features needed to make the first real-terrain
MVP inspectable and repeatable.

Capabilities gained:

- stable named or documented scenario identities
- exports that carry enough run metadata for safe comparison
- compact run summaries that let a user compare outcomes before a viewer exists

Why this phase matters:

- it turns a raw example into a small but usable real-scenario workflow
- it reduces the risk of discussing outputs that cannot be reproduced or even identified correctly

Guardrails:

- do not build a broad scenario-management subsystem yet
- prefer a few explicit, documented workflows over flexible but weakly justified abstractions
- stop once the repository can support one clear comparison workflow on a real clip

## Phase 4. First realism-bearing hydrology

After the minimum real-scenario scaffolding is in place, priority should move
to model features that change practical usefulness rather than only workflow
shape.

Capabilities gained:

- at least one edge-behavior model better suited to clipped real terrain than only closed boundaries
- at least one simple runoff-loss or infiltration representation so rainfall does not map directly to surface water in every case
- clearer tests and benchmarks for how those features change results

Why this phase matters:

- this is the point where FloodSim starts improving model meaning, not just run mechanics
- it is the earliest plausible bridge from a deterministic demo to an MVP with real-world screening value

Expected outcome:

- the repository can run a real terrain clip with basic scenario identity plus a first-pass realism improvement
- contributors can explain what physical approximation was added, what it still omits, and how tests constrain it

## Phase 5. Map visualization

Expose outputs in forms that can be rendered over real maps and tiles.

Capabilities gained:

- flood-depth outputs that can be overlaid on real basemaps
- easier inspection of where water accumulates relative to streets, parcels, and terrain features
- a clearer path to analyst and planner workflows

Why this phase matters:

- flood outputs become much more interpretable once they can be compared to real geography
- this is where technical outputs start becoming usable planning artifacts rather than internal model artifacts

## Phase 6. Graphical viewer

Add a lightweight graphical app for inspecting terrain, water depth, and scenario outputs without changing the MVP focus on a simple simulation core. Start with visualization of exported outputs before considering richer interactive tooling.

Capabilities gained:

- local visual inspection of runs without relying on raw CSV or ad hoc scripts
- faster scenario review for engineering, planning, and product iteration
- a first user-facing interface layer on top of the simulation outputs

Why this phase matters:

- a viewer improves iteration speed and user trust
- it becomes easier to demonstrate the product to non-developer stakeholders

## Phase 7. Rainfall scenarios

Support configurable rainfall events, durations, intensities, and scenario comparison.

Capabilities gained:

- richer event design than a single uniform rainfall setting
- comparative scenario workflows such as baseline vs. intense storm or short burst vs. long event
- better alignment with planning and risk-analysis questions

Why this phase matters:

- planners and insurers care about scenario differences, not only single-run outputs
- this is where the simulator starts answering decision questions rather than only producing physics-like fields

## Phase 8. Urban drainage and buildings

Incorporate simplified urban drainage effects, impervious surfaces, and obstacle/building representations.

Capabilities gained:

- basic representation of urban form rather than terrain alone
- more realistic runoff and flow concentration behavior in built environments
- stronger relevance for city-scale planning, infrastructure screening, and asset risk workflows

Why this phase matters:

- real urban flood behavior depends heavily on drainage, imperviousness, and built obstacles
- this is a major step toward a product that can support practical city and insurance use cases

## Phase 9. Climate-risk scenarios

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
3. repeatable real-scenario workflows
4. first realism-bearing hydrology
5. map-aligned outputs and interpretation
6. urban surface realism

Until the repository has completed the real-terrain workflow plus at least one
realism-bearing modeling step, it is still mainly proving the simulation kernel.

Once Phase 2 through Phase 8 are substantially in place, FloodSim can begin to support real pilot workflows for:

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
path rather than proving it from scratch. The next transition after that
hardening should be toward realism-bearing model changes, not toward a large
scenario-configuration surface built in isolation.

## Anti-vacuum rule

Do not add architecture simply because a fuller future product might need it.
Add structure only when the current MVP workflow cannot stay clear or
repeatable without it.

Concretely:

- a new workflow abstraction should justify what real-terrain task it unblocks now
- scenario mechanics should stay narrow until they support an actual comparison workflow
- once minimal scaffolding exists, new work should bias toward model meaning, validation, and interpretable outputs

## Working style

Use this roadmap for phase-level direction, not as a task tracker. Convert the active phase into a short iteration plan and create tickets only for changes large enough to need discussion, acceptance criteria, or explicit follow-up.

See [docs/planning_workflow.md](/home/sergio/dev/flood-sim/docs/planning_workflow.md) for the proposed roadmap-to-ticket workflow.
