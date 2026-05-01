#pragma once

#include "floodsim/grid.hpp"

namespace floodsim {

struct RainfallScenario {
    double intensity_m_per_hour {0.0};
};

struct SimulationConfig {
    double time_step_seconds {1.0};
    double max_outflow_fraction {0.25};
};

void add_uniform_rainfall(Grid& grid, const RainfallScenario& rainfall, double duration_seconds);
void step(Grid& grid, const RainfallScenario& rainfall, const SimulationConfig& config);

}  // namespace floodsim

