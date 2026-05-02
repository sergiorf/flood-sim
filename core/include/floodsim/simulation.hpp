#pragma once

#include "floodsim/grid.hpp"

namespace floodsim {

struct RainfallScenario {
    // Uniform rainfall intensity applied to every cell during a time step.
    double intensity_m_per_hour {0.0};
};

struct SimulationConfig {
    // Duration of one simulation step.
    double time_step_seconds {1.0};
    // Maximum fraction of a cell's current water depth that may leave in one step.
    double max_outflow_fraction {0.25};
};

void add_uniform_rainfall(Grid& grid, const RainfallScenario& rainfall, double duration_seconds);
void step(Grid& grid, const RainfallScenario& rainfall, const SimulationConfig& config);

}  // namespace floodsim
