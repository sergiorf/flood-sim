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

}  // namespace

void add_uniform_rainfall(Grid& grid, const RainfallScenario& rainfall, double duration_seconds) {
    if (duration_seconds < 0.0) {
        throw std::invalid_argument("Rainfall duration cannot be negative");
    }

    const double added_depth = rainfall.intensity_m_per_hour * (duration_seconds / 3600.0);
    if (added_depth <= 0.0) {
        return;
    }

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
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

    add_uniform_rainfall(grid, rainfall, config.time_step_seconds);

    const std::size_t total_cells = grid.rows() * grid.cols();
    std::vector<double> delta(total_cells, 0.0);

    constexpr std::array<int, 4> d_row { -1, 1, 0, 0 };
    constexpr std::array<int, 4> d_col { 0, 0, -1, 1 };

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            const double available_water = grid.water_depth(row, col);
            if (available_water <= 0.0) {
                continue;
            }

            const double current_surface = grid.surface_height(row, col);
            std::vector<Neighbor> lower_neighbors;
            double total_drop = 0.0;

            for (std::size_t i = 0; i < d_row.size(); ++i) {
                const int neighbor_row = static_cast<int>(row) + d_row[i];
                const int neighbor_col = static_cast<int>(col) + d_col[i];
                if (neighbor_row < 0 || neighbor_col < 0) {
                    continue;
                }
                if (neighbor_row >= static_cast<int>(grid.rows()) ||
                    neighbor_col >= static_cast<int>(grid.cols())) {
                    continue;
                }

                const double neighbor_surface = grid.surface_height(
                    static_cast<std::size_t>(neighbor_row),
                    static_cast<std::size_t>(neighbor_col));

                const double drop = current_surface - neighbor_surface;
                if (drop > 0.0) {
                    lower_neighbors.push_back({
                        static_cast<std::size_t>(neighbor_row),
                        static_cast<std::size_t>(neighbor_col),
                        drop,
                    });
                    total_drop += drop;
                }
            }

            if (lower_neighbors.empty() || total_drop <= 0.0) {
                continue;
            }

            const double outflow = available_water * config.max_outflow_fraction;
            const std::size_t source_idx = row * grid.cols() + col;
            delta[source_idx] -= outflow;

            for (const Neighbor& neighbor : lower_neighbors) {
                const double share = outflow * (neighbor.drop / total_drop);
                const std::size_t neighbor_idx = neighbor.row * grid.cols() + neighbor.col;
                delta[neighbor_idx] += share;
            }
        }
    }

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            const std::size_t idx = row * grid.cols() + col;
            grid.add_water_depth(row, col, delta[idx]);
        }
    }
}

}  // namespace floodsim

