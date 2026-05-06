#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace floodsim {

class Grid;

struct TerrainRaster {
    // Raster dimensions in cells.
    std::size_t rows {0};
    std::size_t cols {0};

    // Square cell size in meters for the first Phase 2 terrain workflow.
    double cell_size_m {0.0};

    // Row-major elevation values in meters, including placeholder entries for
    // nodata cells so indexing stays consistent with the source raster.
    std::vector<double> elevation_m;

    // Row-major validity mask. 1 means the cell is part of the simulation
    // domain, 0 means the cell is nodata / out of domain.
    std::vector<std::uint8_t> valid_cell_mask;

    // Optional real-world placement metadata preserved for later map-aligned
    // export and visualization work.
    std::optional<double> origin_x_m;
    std::optional<double> origin_y_m;
    std::optional<std::string> crs_id;

    [[nodiscard]] std::size_t cell_count() const noexcept;
    [[nodiscard]] std::size_t valid_cell_count() const noexcept;
};

struct TerrainWindow {
    std::size_t row_offset {0};
    std::size_t col_offset {0};
    std::size_t rows {0};
    std::size_t cols {0};
};

void validate_terrain_raster(const TerrainRaster& terrain);
void validate_terrain_window(const TerrainWindow& window);

// Convert validated terrain-contract data into the simulation grid used by the
// current core. This keeps ingestion/file-format concerns separate from
// simulation setup.
Grid make_grid_from_terrain(const TerrainRaster& terrain);

// Load a terrain raster through the first GDAL-backed ingestion path.
// Phase 2 intentionally supports a narrow scope first:
// - one raster band
// - square pixels
// - no rotated/sheared geotransform
// - nodata mapped into valid_cell_mask
TerrainRaster load_terrain_raster_from_file(const std::string& path);
TerrainRaster load_terrain_raster_from_file(const std::string& path, const TerrainWindow& window);

}  // namespace floodsim
