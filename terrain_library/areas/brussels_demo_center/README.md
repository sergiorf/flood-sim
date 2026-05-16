# Brussels Demo Center

## Status

- `area_id`: `brussels_demo_center`
- `display_name`: `Brussels demo center clip`
- `status`: `candidate`
- `intended_use`: `demo`

This is the first planned real-place demo area for FloodSim.

The purpose of this entry is to define the canonical Brussels demo target
before committing to one exact raster and clip boundary.

## Why this area

- It supports the current roadmap priority of a small, recognizable Brussels demo.
- It is easier to explain than an abstract regression fixture.
- It can become the canonical walkthrough area for screening scenarios.

## Source candidates

### Preferred source

- `source_name`: `Brussels regional open data / UrbIS`
- `source_url`: `https://be.brussels/about-region/urbis-data`
- `reason`: official regional source with stronger local credibility than a generic global DEM

### Baseline fallback

- `source_name`: `Copernicus DEM GLO-30`
- `source_url`: `https://dataspace.copernicus.eu/explore-data/data-collections/copernicus-contributing-missions/collections-description/COP-DEM`
- `reason`: easy free baseline for rapid screening and early workflow testing

### Discovery option

- `source_name`: `OpenTopography`
- `source_url`: `https://www.opentopography.org/start`
- `reason`: possible route to better local terrain products if available for Brussels

## Open questions

- Which exact Brussels sub-area should be the canonical demo clip?
- Which source provides the best tradeoff between official provenance and low-friction access?
- Can the first demo area use a committed small derived clip, or should it remain staged-only at first?
- What exact CRS and vertical reference should be preserved in the metadata once the source is selected?

## First acceptance target

This area should move from `candidate` to `active_demo` once FloodSim has:

- one exact clip boundary
- one documented source and license
- one staged or derived raster path
- one repeatable scenario walkthrough
- one visual interpretation path

## Screening limitations

Even once active, this terrain entry supports a screening demo only.

It does not imply:

- regulatory acceptance
- calibrated local hydrology
- validated city-scale flood behavior
