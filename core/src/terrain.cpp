#include "floodsim/terrain.hpp"

#include <algorithm>
#include <stdexcept>

namespace floodsim {

std::size_t TerrainRaster::cell_count() const noexcept {
    return rows * cols;
}

void validate_terrain_raster(const TerrainRaster& terrain) {
    if (terrain.rows == 0 || terrain.cols == 0) {
        throw std::invalid_argument("Terrain raster dimensions must be positive");
    }
    if (terrain.cell_size_m <= 0.0) {
        throw std::invalid_argument("Terrain raster cell size must be positive");
    }

    const std::size_t expected_cells = terrain.cell_count();
    if (terrain.elevation_m.size() != expected_cells) {
        throw std::invalid_argument("Terrain raster elevation array size does not match rows * cols");
    }
    if (terrain.valid_cell_mask.size() != expected_cells) {
        throw std::invalid_argument("Terrain raster valid-cell mask size does not match rows * cols");
    }

    const bool has_origin_x = terrain.origin_x_m.has_value();
    const bool has_origin_y = terrain.origin_y_m.has_value();
    if (has_origin_x != has_origin_y) {
        throw std::invalid_argument("Terrain raster origin metadata must provide both x and y or neither");
    }

    const bool has_any_valid_cell = std::any_of(
        terrain.valid_cell_mask.begin(),
        terrain.valid_cell_mask.end(),
        [](std::uint8_t value) { return value != 0; });
    if (!has_any_valid_cell) {
        throw std::invalid_argument("Terrain raster must contain at least one valid simulation cell");
    }
}

}  // namespace floodsim
