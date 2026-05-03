#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace floodsim {

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
};

void validate_terrain_raster(const TerrainRaster& terrain);

}  // namespace floodsim
