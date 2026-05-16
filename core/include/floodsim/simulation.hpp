#pragma once

#include "floodsim/grid.hpp"

namespace floodsim {

enum class BoundaryMode {
    Closed,
    Open,
};

// Uniform rainfall used by the core step function.
//
// The core still operates on one step at a time. More complex event sequences
// can be represented by calling step() repeatedly with a different uniform
// intensity for each step.
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
    // Fraction of rainfall that becomes immediate surface runoff in the raster
    // model. A value of 1.0 means all rainfall is retained as surface water;
    // lower values approximate simple losses such as infiltration.
    double runoff_coefficient {1.0};
    // Per-cell initial rainfall loss in meters. Rainfall must first satisfy
    // this remaining loss store before any water appears as surface depth.
    // This approximates simple event-start losses such as interception,
    // depression storage, or initial infiltration.
    double initial_loss_m {0.0};
    // Maximum fraction of a cell's current water depth that may leave in one step.
    double max_outflow_fraction {0.25};
    // Boundary handling is explicit. Closed boundaries keep all water inside
    // the grid. Open boundaries allow edge cells to discharge water out of the
    // raster across missing orthogonal neighbors.
    BoundaryMode boundary_mode {BoundaryMode::Closed};
};

// Add one uniform rainfall pulse to the grid without routing. This is exposed
// separately so tests and higher-level workflows can reason about rainfall
// semantics independently of flow transfers.
void add_uniform_rainfall(
    Grid& grid,
    const RainfallScenario& rainfall,
    double duration_seconds,
    double runoff_coefficient = 1.0,
    double initial_loss_m = 0.0);

// Route already-ponded surface water across the grid without adding rainfall.
// This lets higher-level workflows reuse the same deterministic routing logic
// after custom rainfall application paths.
void route_surface_water(Grid& grid, const SimulationConfig& config);

// Execute one full simulation step: rainfall first, then routing.
void step(Grid& grid, const RainfallScenario& rainfall, const SimulationConfig& config);

}  // namespace floodsim
