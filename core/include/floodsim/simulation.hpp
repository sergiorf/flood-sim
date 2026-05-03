#pragma once

#include "floodsim/grid.hpp"

namespace floodsim {

enum class BoundaryMode {
    Closed,
};

struct RainfallScenario {
    // Uniform rainfall intensity applied to every cell.
    // The unit is meters of water depth per hour, independent of the chosen
    // simulation time step. A single step adds:
    //   intensity_m_per_hour * (time_step_seconds / 3600.0)
    double intensity_m_per_hour {0.0};
};

struct SimulationConfig {
    // Duration of one simulation step. Rainfall input is scaled by this value
    // before any flow routing happens during the step.
    double time_step_seconds {1.0};
    // Maximum fraction of a cell's current water depth that may leave in one step.
    double max_outflow_fraction {0.25};
    // Phase 1 currently supports only closed boundaries: water may move only
    // to in-domain neighbors and cannot leave the raster across an edge.
    BoundaryMode boundary_mode {BoundaryMode::Closed};
};

void add_uniform_rainfall(Grid& grid, const RainfallScenario& rainfall, double duration_seconds);
void step(Grid& grid, const RainfallScenario& rainfall, const SimulationConfig& config);

}  // namespace floodsim
