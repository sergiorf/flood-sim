#include "floodsim/simulation.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace floodsim {

namespace {

struct Neighbor {
    std::size_t row;
    std::size_t col;
    double drop;
};

struct OpenBoundaryReceiver {
    double drop;
};

struct StepScratch {
    // Reuse the same delta buffer across calls to avoid per-step allocation
    // churn while keeping the step logic snapshot-based and easy to read.
    std::vector<double> delta;
};

StepScratch& step_scratch() {
    thread_local StepScratch scratch;
    return scratch;
}

}  // namespace

void add_uniform_rainfall(Grid& grid, const RainfallScenario& rainfall, double duration_seconds) {
    if (duration_seconds < 0.0) {
        throw std::invalid_argument("Rainfall duration cannot be negative");
    }

    // RainfallScenario stores an intensity, not a per-step depth. Convert it
    // into a depth increment for this specific step duration.
    const double added_depth = rainfall.intensity_m_per_hour * (duration_seconds / 3600.0);
    if (added_depth <= 0.0) {
        return;
    }

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            if (!grid.is_cell_valid(row, col)) {
                continue;
            }

            grid.add_water_depth(row, col, added_depth);
        }
    }
}

void step(Grid& grid, const RainfallScenario& rainfall, const SimulationConfig& config) {
    if (config.time_step_seconds <= 0.0) {
        throw std::invalid_argument("Simulation time step must be positive");
    }
    if (config.max_outflow_fraction < 0.0 || config.max_outflow_fraction > 1.0) {
        throw std::invalid_argument("Max outflow fraction must be in [0, 1]");
    }
    if (config.boundary_mode != BoundaryMode::Closed &&
        config.boundary_mode != BoundaryMode::Open) {
        throw std::invalid_argument("Unsupported boundary mode");
    }

    // Each step first injects rainfall, then redistributes water already on the grid.
    add_uniform_rainfall(grid, rainfall, config.time_step_seconds);

    const std::size_t rows = grid.rows();
    const std::size_t cols = grid.cols();
    const std::size_t total_cells = rows * cols;
    // Accumulate all per-cell water changes here so flow is based on the same snapshot.
    StepScratch& scratch = step_scratch();
    scratch.delta.assign(total_cells, 0.0);
    std::vector<double>& delta = scratch.delta;

    // Only the 4 orthogonal neighbors participate in the current MVP flow model.
    constexpr std::array<int, 4> d_row { -1, 1, 0, 0 };
    constexpr std::array<int, 4> d_col { 0, 0, -1, 1 };

    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t col = 0; col < cols; ++col) {
            if (!grid.is_cell_valid(row, col)) {
                continue;
            }

            const double available_water = grid.water_depth(row, col);
            if (available_water <= 0.0) {
                continue;
            }

            // Surface height is terrain elevation plus ponded water depth.
            const double current_surface = grid.surface_height(row, col);
            std::array<Neighbor, 4> lower_neighbors {};
            std::size_t lower_neighbor_count = 0;
            std::array<OpenBoundaryReceiver, 4> open_receivers {};
            std::size_t open_receiver_count = 0;
            double total_drop = 0.0;

            for (std::size_t i = 0; i < d_row.size(); ++i) {
                const int neighbor_row = static_cast<int>(row) + d_row[i];
                const int neighbor_col = static_cast<int>(col) + d_col[i];
                if (neighbor_row < 0 || neighbor_col < 0) {
                    if (config.boundary_mode == BoundaryMode::Open) {
                        open_receivers[open_receiver_count++] = { available_water };
                        total_drop += available_water;
                    }
                    continue;
                }
                if (neighbor_row >= static_cast<int>(rows) ||
                    neighbor_col >= static_cast<int>(cols)) {
                    if (config.boundary_mode == BoundaryMode::Open) {
                        open_receivers[open_receiver_count++] = { available_water };
                        total_drop += available_water;
                    }
                    continue;
                }
                if (!grid.is_cell_valid(
                        static_cast<std::size_t>(neighbor_row),
                        static_cast<std::size_t>(neighbor_col))) {
                    continue;
                }

                const double neighbor_surface = grid.surface_height(
                    static_cast<std::size_t>(neighbor_row),
                    static_cast<std::size_t>(neighbor_col));

                // Only send water to neighbors with a lower surface.
                const double drop = current_surface - neighbor_surface;
                if (drop > 0.0) {
                    lower_neighbors[lower_neighbor_count++] = {
                        static_cast<std::size_t>(neighbor_row),
                        static_cast<std::size_t>(neighbor_col),
                        drop,
                    };
                    total_drop += drop;
                }
            }

            if ((lower_neighbor_count == 0 && open_receiver_count == 0) || total_drop <= 0.0) {
                continue;
            }

            // Limit how much water can leave this cell during one time step.
            const double outflow = available_water * config.max_outflow_fraction;
            const std::size_t source_idx = row * cols + col;
            delta[source_idx] -= outflow;

            for (std::size_t i = 0; i < lower_neighbor_count; ++i) {
                const Neighbor& neighbor = lower_neighbors[i];
                // Split the outflow proportionally: steeper drops receive more water.
                const double share = outflow * (neighbor.drop / total_drop);
                const std::size_t neighbor_idx = neighbor.row * cols + neighbor.col;
                delta[neighbor_idx] += share;
            }

            // Shares assigned to open receivers leave the grid and therefore do
            // not add to any in-domain delta entry.
        }
    }

    // Apply the accumulated changes after all cells have computed their outflow.
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t col = 0; col < cols; ++col) {
            if (!grid.is_cell_valid(row, col)) {
                continue;
            }

            const std::size_t idx = row * cols + col;
            grid.add_water_depth(row, col, delta[idx]);
        }
    }
}

}  // namespace floodsim
