# Terrain Library Protocol

This document defines how FloodSim should collect, store, and describe real
terrain assets for demos, screening workflows, and regression fixtures.

The goal is not to accumulate arbitrary DEM files. The goal is to build a
small, reviewable terrain library with explicit provenance and intended use.

## Purpose

The terrain library exists to support:

- one trusted demo area for near-term product walkthroughs
- a small set of reviewed screening areas
- repeatable regression fixtures for ingestion and workflow testing
- later expansion into a broader curated library of free terrain sources

The library should stay aligned with the product posture:

- screening-first
- explicit provenance
- narrow local workflows before broad ingestion automation

## Design principles

- Prefer a few trusted clips over a large, weakly documented archive.
- Keep demo assets separate from tiny engine regression fixtures.
- Store enough metadata to explain where a terrain came from and why it is in the repository.
- Preserve source and license identity even when FloodSim later caches or clips a raster locally.
- Avoid pretending a terrain is regulator-ready just because it came from an official source.

## Library layout

```text
terrain_library/
  README.md
  catalog.csv
  areas/
    <area_id>/
      README.md
      source_notes.md           optional
      boundary/                 optional vector or reference files
      staged/                   local staged source rasters, gitignored if large
      derived/                  clipped or normalized local outputs, gitignored if large
```

Notes:

- `areas/<area_id>/README.md` is the canonical human-readable description.
- `catalog.csv` is the small machine-readable index for quick scanning.
- Large staged or derived rasters should not be committed casually.
- Tiny committed fixtures still belong under `examples/real_terrain/data/` when their primary role is regression coverage, not library curation.

## Area ID convention

Use stable lowercase IDs with underscores:

- `brussels_demo_center`
- `brussels_valley_clip`
- `sao_paulo_demo_basin`

Keep IDs short, geographic, and durable.

## Minimum metadata for every area

Every terrain-library area should document:

- `area_id`
- `display_name`
- `city_or_region`
- `country`
- `intended_use`
  Allowed examples: `demo`, `screening`, `regression_fixture`, `research`
- `status`
  Allowed examples: `candidate`, `reviewed`, `active_demo`, `archived`
- `terrain_type`
  Allowed examples: `DEM`, `DTM`, `DSM`
- `source_name`
- `source_url`
- `license_name`
- `acquisition_or_publication_date` when known
- `resolution`
- `crs`
- `vertical_datum` when known
- `bbox_or_clip_description`
- `clip_method`
- `local_files`
- `quality_notes`
- `screening_limitations`

## Review levels

Use these levels consistently:

- `candidate`
  Source identified, but not yet trusted for product use.
- `reviewed`
  Source, license, CRS, and basic clip behavior checked.
- `active_demo`
  Chosen for the current canonical demo path.
- `archived`
  Kept for reference, but not part of the active workflow.

## City onboarding protocol

This protocol defines how FloodSim should onboard one real city area into the
screening product.

The goal is not to make every city fully modeled immediately. The goal is to
make one area repeatable, explainable, and reviewable enough for product demos
and early screening workflows.

### Onboarding stages

Every city area should move through these stages:

1. `candidate`
   One area is identified, but source quality, licensing, or layer coverage is
   still incomplete.
2. `reviewed`
   Terrain source, basic provenance, CRS, and local staging path are checked.
3. `active_demo`
   The area has one documented workflow path from input layers to viewer
   inspection and simulation output.
4. `archived`
   The area remains documented, but is no longer part of the active demo path.

### Required layers for a demo-ready city area

These are the minimum layers or artifacts needed for a credible FloodSim demo
area:

- `terrain`
  One reviewed `DEM`, `DTM`, or `DSM` source with staging metadata
- `scenario set`
  At least one documented screening scenario, preferably with a small named set
- `viewer path`
  One repeatable way to inspect the terrain and result outputs
- `provenance`
  Source name, URL, license, and local file references

### Optional but preferred layers

These are not required for the first terrain-only onboarding milestone, but
they should be tracked explicitly as the next realism layers:

- `buildings`
  footprints, barriers, or obstacle-like built-form layers
- `land_cover`
  pervious or impervious classes, or richer land-cover groupings
- `drainage`
  outlets, sinks, inlets, culverts, or other drainage-relevant features
- `roads_or_parcels`
  useful for interpretation even before they affect the simulation directly

### Source selection order

When several source families exist, choose in this order unless there is a
clear reason not to:

1. official city or regional open data
2. official national or continental open data
3. stable global screening sources
4. community sources such as OSM when official alternatives are absent or too weak

For each layer, document:

- chosen source
- fallback source
- why the chosen source won

### Alignment rules

All area onboarding must document:

- target area identifier
- target CRS for the area package
- extent or clip boundary
- whether each layer is currently raster, vector, or staged-only
- whether each layer is already aligned, or still needs clipping/rasterization

For the current MVP:

- exact reprojection pipelines do not need to be automated yet
- but CRS mismatches must be recorded explicitly
- “unknown” is acceptable during `candidate` status, but should be reduced by `reviewed`

### Demo-readiness checklist

An area may move to `active_demo` only when all of these are true:

- one exact clip boundary is documented
- one terrain source is staged and reviewable locally
- source and license fields are filled in
- at least one scenario walkthrough exists
- the native viewer path can inspect the staged terrain or result output
- screening limitations are written clearly

### Area README protocol

Every `terrain_library/areas/<area_id>/README.md` should contain these sections:

- `Status`
- `Why this area`
- `Layer inventory`
- `Source candidates`
- `Open questions`
- `First acceptance target`
- `Screening limitations`

### Layer inventory section

The `Layer inventory` section should list the layer state explicitly.

Suggested format:

```text
## Layer inventory

- terrain: present, candidate, staged locally
- buildings: missing
- land_cover: missing
- drainage: missing
- scenarios: missing
- viewer_path: present
```

### Catalog expectations

`terrain_library/catalog.csv` is the machine-readable summary, not the full
protocol narrative.

Each row should:

- point to a valid area `README.md` when it represents a real area
- use a valid `status`
- use a valid `intended_use`
- use a valid `terrain_type`

### Validation tooling

The repository now includes one lightweight validator:

```bash
python3 data/scripts/validate_terrain_library.py
```

That validator currently checks:

- `catalog.csv` header shape
- allowed values for `status`, `intended_use`, and `terrain_type`
- existence of referenced local README paths
- presence of the required top-level sections in each area README

It is intentionally narrow. It does not yet validate deeper layer semantics or
source licensing text quality.

## Recommended free-source starting points

As of May 16, 2026, the most practical initial free-source set for FloodSim is:

- `Brussels regional open data / UrbIS / Datastore`
  Good first stop for Brussels-specific official geographic data.
  Source: `https://be.brussels/about-region/urbis-data`
- `Copernicus DEM GLO-30`
  Strong baseline free DEM source for Europe and global screening coverage.
  Source: `https://dataspace.copernicus.eu/explore-data/data-collections/copernicus-contributing-missions/collections-description/COP-DEM`
- `OpenTopography`
  Useful discovery and download portal for free topographic and lidar-backed datasets where available.
  Source: `https://www.opentopography.org/start`
- `USGS EarthExplorer / SRTM`
  Practical global fallback when better local terrain is unavailable.
  Sources:
  `https://www.usgs.gov/tools/earthexplorer`
  `https://www.usgs.gov/centers/eros/science/usgs-eros-archive-digital-elevation-shuttle-radar-topography-mission-srtm-1-arc`

For Brussels, preferred source order should be:

1. official Brussels regional open data
2. Copernicus DEM baseline fallback
3. OpenTopography discovery path if better local products are available

## Intake workflow

When adding a new terrain:

1. Create `terrain_library/areas/<area_id>/README.md`.
2. Add one row to `terrain_library/catalog.csv`.
3. Record the exact source URL and license name.
4. Record whether the terrain is a `DEM`, `DTM`, or `DSM`.
5. Record the intended use and review status.
6. Only then add any staged or derived files locally.

## Locking a canonical clip and terrain source

Before adding more layers or scenarios for a city demo area, lock one exact
terrain procurement decision first.

Use this procedure:

1. Choose one exact clip boundary and record it in the area README.
2. Choose one canonical terrain source URL and license to cite in docs.
3. Stage one local raster from that source, or from a documented mirror that
   preserves the same provenance clearly.
4. Record whether the staged file is the canonical source itself or a
   convenience mirror.
5. Keep alternative sources as fallbacks or upgrade candidates, not as
   competing “current” truths.

For the current Brussels demo area, the locked first-pass staging window is:

- south: `50.80`
- north: `50.90`
- west: `4.30`
- east: `4.42`

For that area, the current baseline terrain choice should be:

- canonical source family: `Copernicus DEM GLO-30`
- rationale: official continental source, stable public access path, clear
  licensing, and already aligned with the repository's staged-download helper
- upgrade path: replace the baseline only when one exact Brussels-native UrbIS
  elevation dataset is identified, licensed, and reviewed for the same clip

`OpenTopography` remains useful as a staging or comparison path, but the area
README should still cite the direct underlying source choice that FloodSim is
claiming for the demo package.

## Download automation

The repository now includes a small staging helper:

`data/scripts/download_terrain.py`

Current automation scope:

- `OpenTopography` global datasets API
- `Copernicus DEM` process API
- direct `http` / `https` file download for documented source URLs

Example OpenTopography call:

```bash
python3 data/scripts/download_terrain.py opentopography \
  --area-id brussels_demo_center \
  --dataset EU_DTM \
  --south 50.80 \
  --north 50.90 \
  --west 4.30 \
  --east 4.42 \
  --filename brussels_eu_dtm.tif \
  --metadata-json terrain_library/areas/brussels_demo_center/staged/brussels_eu_dtm.json
```

Example Copernicus DEM call:

```bash
python3 data/scripts/download_terrain.py copernicus-dem \
  --area-id brussels_demo_center \
  --dem-instance COPERNICUS_30 \
  --south 50.80 \
  --north 50.90 \
  --west 4.30 \
  --east 4.42 \
  --filename brussels_copernicus_30.tif \
  --metadata-json terrain_library/areas/brussels_demo_center/staged/brussels_copernicus_30.json
```

Credential notes:

- OpenTopography requires a free API key through `OPENTOPOGRAPHY_API_KEY` or `--api-key`.
- Copernicus DEM requires OAuth client credentials through `COPERNICUS_CLIENT_ID` and `COPERNICUS_CLIENT_SECRET` or explicit flags.

Current limit:

- Brussels regional open-data downloads are still dataset-specific. Until one exact Brussels terrain dataset and stable endpoint are selected, those sources should remain documented and staged manually or through `download-url`.

## Current download locations

For the Brussels demo effort, these are the practical download starting points:

- `Copernicus DEM GLO-30`
  Canonical baseline terrain source for the current demo package.
  Official entry point:
  `https://dataspace.copernicus.eu/explore-data/data-collections/copernicus-contributing-missions/collections-description/COP-DEM`
- `Brussels regional open data / UrbIS`
  Preferred family for later Brussels-specific upgrades and additional city
  layers such as buildings or land-cover data.
  Regional overview:
  `https://be.brussels/en/about-region/urbis-data`
- `Brussels Datastore`
  Regional portal where dataset-specific downloads and service endpoints are
  published.
  Portal:
  `https://be.brussels/en/about-region/datastore`
- `OpenTopography`
  Useful convenience portal when testing alternate free DEM sources or using
  the repository's existing OpenTopography downloader path.
  Portal:
  `https://www.opentopography.org/start`

## Demo guardrail

For the current FloodSim milestone, the terrain library should optimize for:

- one small trusted Brussels demo area
- a few documented candidate areas for later expansion
- explicit screening disclaimers

It should not yet become a bulk downloader, a national terrain mirror, or a
general GIS asset warehouse.
