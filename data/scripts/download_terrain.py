#!/usr/bin/env python3
"""
Download staged terrain rasters for the FloodSim terrain library.

This script intentionally starts narrow:

- OpenTopography global DEM API
- Copernicus Data Space Sentinel Hub DEM process API
- direct URL download for documented source files

The script does not try to be a full terrain catalog manager. It is a small
staging helper for reviewed or candidate terrain-library entries.
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
TERRAIN_LIBRARY_ROOT = REPO_ROOT / "terrain_library" / "areas"

OPENTOPOGRAPHY_ENDPOINT = "https://portal.opentopography.org/API/globaldem"
COPERNICUS_TOKEN_ENDPOINT = (
    "https://identity.dataspace.copernicus.eu/auth/realms/CDSE/protocol/openid-connect/token"
)
COPERNICUS_PROCESS_ENDPOINT = "https://sh.dataspace.copernicus.eu/process/v1"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Download staged terrain rasters for FloodSim."
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    opentopo = subparsers.add_parser(
        "opentopography",
        help="Download a DEM subset through the OpenTopography Global Datasets API.",
    )
    add_common_output_args(opentopo)
    add_bbox_args(opentopo)
    opentopo.add_argument(
        "--dataset",
        default="COP30",
        help=(
            "OpenTopography dataset code, for example COP30, COP90, EU_DTM, "
            "SRTMGL1, NASADEM."
        ),
    )
    opentopo.add_argument(
        "--output-format",
        default="GTiff",
        choices=["GTiff", "AAIGrid", "HFA"],
        help="Requested OpenTopography output format.",
    )
    opentopo.add_argument(
        "--api-key",
        default=os.environ.get("OPENTOPOGRAPHY_API_KEY"),
        help="OpenTopography API key. Defaults to OPENTOPOGRAPHY_API_KEY.",
    )

    cop_dem = subparsers.add_parser(
        "copernicus-dem",
        help="Download a DEM subset through the Copernicus DEM process API.",
    )
    add_common_output_args(cop_dem)
    add_bbox_args(cop_dem)
    cop_dem.add_argument(
        "--dem-instance",
        default="COPERNICUS_30",
        choices=["COPERNICUS_30", "COPERNICUS_90"],
        help="Copernicus DEM instance to request.",
    )
    cop_dem.add_argument(
        "--resx",
        type=float,
        default=0.0003,
        help="Output x resolution in geographic degrees for the process request.",
    )
    cop_dem.add_argument(
        "--resy",
        type=float,
        default=0.0003,
        help="Output y resolution in geographic degrees for the process request.",
    )
    cop_dem.add_argument(
        "--egm",
        action="store_true",
        help="Request ellipsoidal heights by enabling EGM processing.",
    )
    cop_dem.add_argument(
        "--client-id",
        default=os.environ.get("COPERNICUS_CLIENT_ID"),
        help="Copernicus OAuth client id. Defaults to COPERNICUS_CLIENT_ID.",
    )
    cop_dem.add_argument(
        "--client-secret",
        default=os.environ.get("COPERNICUS_CLIENT_SECRET"),
        help="Copernicus OAuth client secret. Defaults to COPERNICUS_CLIENT_SECRET.",
    )

    direct = subparsers.add_parser(
        "download-url",
        help="Download a documented terrain source from a direct URL.",
    )
    add_common_output_args(direct)
    direct.add_argument("--url", required=True, help="Direct file URL to download.")

    return parser.parse_args()


def add_common_output_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--area-id",
        help=(
            "Terrain-library area id. When set, default output goes under "
            "terrain_library/areas/<area-id>/staged/ unless --output is provided."
        ),
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Explicit output path for the staged raster.",
    )
    parser.add_argument(
        "--filename",
        help="Output filename when using --area-id without --output.",
    )
    parser.add_argument(
        "--metadata-json",
        type=Path,
        help="Optional sidecar JSON path describing the staged download.",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing output files.",
    )


def add_bbox_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--south", required=True, type=float, help="South latitude.")
    parser.add_argument("--north", required=True, type=float, help="North latitude.")
    parser.add_argument("--west", required=True, type=float, help="West longitude.")
    parser.add_argument("--east", required=True, type=float, help="East longitude.")


def main() -> int:
    args = parse_args()
    try:
        if args.command == "opentopography":
            return handle_opentopography(args)
        if args.command == "copernicus-dem":
            return handle_copernicus_dem(args)
        if args.command == "download-url":
            return handle_direct_download(args)
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    print(f"error: unhandled command {args.command}", file=sys.stderr)
    return 1


def handle_opentopography(args: argparse.Namespace) -> int:
    if not args.api_key:
        raise RuntimeError(
            "OpenTopography requires an API key. Set OPENTOPOGRAPHY_API_KEY or pass --api-key."
        )

    output_path = resolve_output_path(
        args.output,
        args.area_id,
        args.filename or default_filename("opentopography", args.dataset, "tif"),
        args.force,
    )
    query = urllib.parse.urlencode(
        {
            "demtype": args.dataset,
            "south": args.south,
            "north": args.north,
            "west": args.west,
            "east": args.east,
            "outputFormat": args.output_format,
            "API_Key": args.api_key,
        }
    )
    url = f"{OPENTOPOGRAPHY_ENDPOINT}?{query}"
    metadata_url = build_url(
        OPENTOPOGRAPHY_ENDPOINT,
        {
            "demtype": args.dataset,
            "south": args.south,
            "north": args.north,
            "west": args.west,
            "east": args.east,
            "outputFormat": args.output_format,
            "API_Key": "<redacted>",
        },
    )
    download_binary(url, output_path)
    maybe_write_metadata(
        args.metadata_json,
        {
            "source": "opentopography",
            "dataset": args.dataset,
            "endpoint": OPENTOPOGRAPHY_ENDPOINT,
            "request_url": metadata_url,
            "south": args.south,
            "north": args.north,
            "west": args.west,
            "east": args.east,
            "output_path": str(output_path),
        },
        args.force,
    )
    print(f"downloaded={output_path}")
    return 0


def handle_copernicus_dem(args: argparse.Namespace) -> int:
    if not args.client_id or not args.client_secret:
        raise RuntimeError(
            "Copernicus DEM requires OAuth client credentials. "
            "Set COPERNICUS_CLIENT_ID and COPERNICUS_CLIENT_SECRET or pass them explicitly."
        )

    output_path = resolve_output_path(
        args.output,
        args.area_id,
        args.filename or default_filename("copernicus", args.dem_instance.lower(), "tif"),
        args.force,
    )
    access_token = request_copernicus_access_token(args.client_id, args.client_secret)
    request_body = build_copernicus_dem_request(
        west=args.west,
        south=args.south,
        east=args.east,
        north=args.north,
        dem_instance=args.dem_instance,
        resx=args.resx,
        resy=args.resy,
        egm=args.egm,
    )
    headers = {
        "Authorization": f"Bearer {access_token}",
        "Content-Type": "application/json",
    }
    download_binary(
        COPERNICUS_PROCESS_ENDPOINT,
        output_path,
        method="POST",
        headers=headers,
        body=json.dumps(request_body).encode("utf-8"),
    )
    maybe_write_metadata(
        args.metadata_json,
        {
            "source": "copernicus-dem",
            "dem_instance": args.dem_instance,
            "endpoint": COPERNICUS_PROCESS_ENDPOINT,
            "south": args.south,
            "north": args.north,
            "west": args.west,
            "east": args.east,
            "resx": args.resx,
            "resy": args.resy,
            "egm": args.egm,
            "output_path": str(output_path),
        },
        args.force,
    )
    print(f"downloaded={output_path}")
    return 0


def handle_direct_download(args: argparse.Namespace) -> int:
    parsed_url = urllib.parse.urlparse(args.url)
    if parsed_url.scheme not in {"http", "https"}:
        raise RuntimeError("Direct URL downloads require an http or https URL")

    inferred_name = Path(parsed_url.path).name or "terrain_source.bin"
    output_path = resolve_output_path(
        args.output,
        args.area_id,
        args.filename or inferred_name,
        args.force,
    )
    download_binary(args.url, output_path)
    maybe_write_metadata(
        args.metadata_json,
        {
            "source": "direct-url",
            "url": args.url,
            "output_path": str(output_path),
        },
        args.force,
    )
    print(f"downloaded={output_path}")
    return 0


def build_url(base_url: str, query_params: dict[str, Any]) -> str:
    return f"{base_url}?{urllib.parse.urlencode(query_params)}"


def resolve_output_path(
    explicit_output: Path | None,
    area_id: str | None,
    filename: str | None,
    force: bool,
) -> Path:
    if explicit_output is not None:
        output_path = explicit_output
    else:
        if not area_id:
            raise RuntimeError("Provide either --output or --area-id")
        if not filename:
            raise RuntimeError("A filename is required when using --area-id without --output")
        output_path = TERRAIN_LIBRARY_ROOT / area_id / "staged" / filename

    output_path.parent.mkdir(parents=True, exist_ok=True)
    if output_path.exists() and not force:
        raise RuntimeError(
            f"Output already exists: {output_path}. Use --force to overwrite."
        )
    return output_path


def default_filename(prefix: str, dataset: str, suffix: str) -> str:
    safe_dataset = dataset.lower().replace("/", "_").replace("-", "_")
    return f"{prefix}_{safe_dataset}.{suffix}"


def download_binary(
    url: str,
    output_path: Path,
    *,
    method: str = "GET",
    headers: dict[str, str] | None = None,
    body: bytes | None = None,
) -> None:
    request = urllib.request.Request(
        url,
        data=body,
        headers=headers or {},
        method=method,
    )
    try:
        with urllib.request.urlopen(request) as response, output_path.open("wb") as handle:
            handle.write(response.read())
    except urllib.error.HTTPError as error:
        detail = error.read().decode("utf-8", errors="replace")
        raise RuntimeError(
            f"HTTP {error.code} while downloading {url}: {detail}"
        ) from error
    except urllib.error.URLError as error:
        raise RuntimeError(f"Failed to reach {url}: {error.reason}") from error


def request_copernicus_access_token(client_id: str, client_secret: str) -> str:
    payload = urllib.parse.urlencode(
        {
            "grant_type": "client_credentials",
            "client_id": client_id,
            "client_secret": client_secret,
        }
    ).encode("utf-8")
    request = urllib.request.Request(
        COPERNICUS_TOKEN_ENDPOINT,
        data=payload,
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(request) as response:
            token_response = json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as error:
        detail = error.read().decode("utf-8", errors="replace")
        raise RuntimeError(
            f"Copernicus token request failed with HTTP {error.code}: {detail}"
        ) from error
    except urllib.error.URLError as error:
        raise RuntimeError(
            f"Failed to reach Copernicus token endpoint: {error.reason}"
        ) from error

    access_token = token_response.get("access_token")
    if not access_token:
        raise RuntimeError("Copernicus token response did not include an access_token")
    return access_token


def build_copernicus_dem_request(
    *,
    west: float,
    south: float,
    east: float,
    north: float,
    dem_instance: str,
    resx: float,
    resy: float,
    egm: bool,
) -> dict[str, Any]:
    return {
        "input": {
            "bounds": {
                "properties": {"crs": "http://www.opengis.net/def/crs/OGC/1.3/CRS84"},
                "bbox": [west, south, east, north],
            },
            "data": [
                {
                    "type": "dem",
                    "dataFilter": {"demInstance": dem_instance},
                    "processing": {
                        "upsampling": "BILINEAR",
                        "downsampling": "BILINEAR",
                        "egm": egm,
                    },
                }
            ],
        },
        "output": {
            "resx": resx,
            "resy": resy,
            "responses": [
                {
                    "identifier": "default",
                    "format": {"type": "image/tiff"},
                }
            ],
        },
        "evalscript": (
            "//VERSION=3\n"
            "function setup() {\n"
            "  return {\n"
            "    input: [\"DEM\"],\n"
            "    output: { id: \"default\", bands: 1, sampleType: SampleType.FLOAT32 },\n"
            "  };\n"
            "}\n"
            "function evaluatePixel(sample) {\n"
            "  return [sample.DEM];\n"
            "}\n"
        ),
    }


def maybe_write_metadata(
    metadata_path: Path | None,
    metadata: dict[str, Any],
    force: bool,
) -> None:
    if metadata_path is None:
        return
    metadata_path.parent.mkdir(parents=True, exist_ok=True)
    if metadata_path.exists() and not force:
        raise RuntimeError(
            f"Metadata file already exists: {metadata_path}. Use --force to overwrite."
        )
    metadata_path.write_text(json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8")


if __name__ == "__main__":
    sys.exit(main())
