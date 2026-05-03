#pragma once

#include "floodsim/grid.hpp"

#include <iosfwd>

namespace floodsim {

// Write a text CSV export with a short metadata preamble followed by one data
// row per cell. The preamble keeps the Phase 1 output self-describing without
// introducing a separate sidecar file or binary container.
void write_grid_csv(const Grid& grid, std::ostream& output);

}  // namespace floodsim
