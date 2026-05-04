#pragma once

#include "floodsim/grid.hpp"

#include <iosfwd>
#include <optional>
#include <string>

namespace floodsim {

struct GridCsvMetadata {
    std::optional<double> origin_x_m;
    std::optional<double> origin_y_m;
    std::optional<std::string> crs_id;
};

// Write a text CSV export with a short metadata preamble followed by one data
// row per cell. The preamble keeps the Phase 1 output self-describing without
// introducing a separate sidecar file or binary container. Terrain-derived
// runs can attach origin / CRS metadata without changing the per-cell layout.
void write_grid_csv(
    const Grid& grid,
    std::ostream& output,
    const GridCsvMetadata& metadata = {});

}  // namespace floodsim
