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

## Demo guardrail

For the current FloodSim milestone, the terrain library should optimize for:

- one small trusted Brussels demo area
- a few documented candidate areas for later expansion
- explicit screening disclaimers

It should not yet become a bulk downloader, a national terrain mirror, or a
general GIS asset warehouse.
