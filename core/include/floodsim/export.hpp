#pragma once

#include "floodsim/grid.hpp"

#include <iosfwd>

namespace floodsim {

// Write one CSV row per cell so downstream tools can reconstruct the raster
// without needing a custom binary format in the Phase 1 MVP.
void write_grid_csv(const Grid& grid, std::ostream& output);

}  // namespace floodsim
