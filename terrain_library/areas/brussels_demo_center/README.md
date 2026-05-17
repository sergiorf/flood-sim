# Brussels Demo Center

## Status

- `area_id`: `brussels_demo_center`
- `display_name`: `Brussels demo center clip`
- `status`: `candidate`
- `intended_use`: `demo`
- `locked_bbox_wgs84`: `south=50.80, north=50.90, west=4.30, east=4.42`

This is the first planned real-place demo area for FloodSim.

The purpose of this entry is now to lock the first canonical Brussels demo
terrain window and baseline source before expanding the area into a fuller
layered demo package.

## Why this area

- It supports the current roadmap priority of a small, recognizable Brussels demo.
- It is easier to explain than an abstract regression fixture.
- It can become the canonical walkthrough area for screening scenarios.

## Layer inventory

- terrain: present, baseline source chosen, staged locally for review
- buildings: missing
- land_cover: missing
- drainage: missing
- scenarios: missing
- viewer_path: present through the native viewer scaffold

## Source candidates

### Current locked baseline terrain source

- `source_name`: `Copernicus DEM GLO-30`
- `source_url`: `https://dataspace.copernicus.eu/explore-data/data-collections/copernicus-contributing-missions/collections-description/COP-DEM`
- `reason`: official continental source with a stable public access path, clear citation rules, and an existing repo download path
- `download_path`: Copernicus Browser or API, or the repo helper `python3 data/scripts/download_terrain.py copernicus-dem ...`
- `terrain_type`: `DEM`
- `resolution`: `30 m`
- `source_crs`: `EPSG:4326` horizontal, `EPSG:3855` vertical at source

### Preferred Brussels-specific upgrade path

- `source_name`: `Brussels regional open data / UrbIS`
- `source_url`: `https://be.brussels/en/about-region/urbis-data`
- `reason`: official regional source with stronger local credibility for a Brussels-focused demo once one exact elevation dataset and download endpoint are reviewed

### Baseline fallback

- `source_name`: `OpenTopography`
- `source_url`: `https://www.opentopography.org/start`
- `reason`: useful convenience portal for staging or comparing free DEM products over the same locked Brussels clip

### Discovery option

- `source_name`: `Brussels Datastore`
- `source_url`: `https://be.brussels/en/about-region/datastore`
- `reason`: regional dataset portal for reviewing exact Brussels download endpoints for later terrain upgrades and other city layers

## Download procedure

Lock and stage this area in the following order:

1. Keep the current WGS84 clip window:
   `south=50.80`, `north=50.90`, `west=4.30`, `east=4.42`
2. Treat `Copernicus DEM GLO-30` as the canonical baseline terrain source for
   the first demo package.
3. Stage the raster locally with the existing repo helper:

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

4. Record source, license, and any derived clip notes in this README.
5. Only replace the baseline when one exact UrbIS elevation dataset is
   identified and reviewed against the same clip.

Current staged baseline files:

- `terrain_library/areas/brussels_demo_center/staged/brussels_copernicus_30.tif`
- `terrain_library/areas/brussels_demo_center/staged/brussels_copernicus_30.json`

The currently staged `brussels_eu_dtm.tif` file is acceptable as a local
comparison or convenience artifact, but it should not be treated as the
canonical cited source for the demo package.

## Open questions

- Should the locked staging window later shrink to a smaller inner Brussels
  presentation clip while keeping the same procurement source?
- Which exact UrbIS elevation dataset, if any, is strong enough to replace the
  Copernicus baseline for the canonical demo path?
- Should the first demo area use a committed small derived clip, or remain
  staged-only?
- What exact downstream target CRS should be standardized across the terrain
  and later Brussels-specific layers?

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
