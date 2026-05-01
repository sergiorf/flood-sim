#include "floodsim/grid.hpp"
#include "floodsim/simulation.hpp"

#include <iomanip>
#include <iostream>

namespace {

void initialize_bowl_terrain(floodsim::Grid& grid) {
    const double center_row = static_cast<double>(grid.rows() - 1) / 2.0;
    const double center_col = static_cast<double>(grid.cols() - 1) / 2.0;

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            const double dr = static_cast<double>(row) - center_row;
            const double dc = static_cast<double>(col) - center_col;
            const double elevation = 0.1 * (dr * dr + dc * dc);
            grid.set_elevation(row, col, elevation);
        }
    }
}

void print_water_depth(const floodsim::Grid& grid) {
    std::cout << "Final water depth grid (m)\n";
    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            std::cout << std::fixed << std::setprecision(3) << grid.water_depth(row, col) << ' ';
        }
        std::cout << '\n';
    }
}

}  // namespace

int main() {
    floodsim::Grid grid(10, 10, 2.0);
    initialize_bowl_terrain(grid);

    floodsim::RainfallScenario rainfall {
        .intensity_m_per_hour = 0.018,
    };
    floodsim::SimulationConfig config {
        .time_step_seconds = 60.0,
        .max_outflow_fraction = 0.20,
    };

    for (int step = 0; step < 10; ++step) {
        floodsim::step(grid, rainfall, config);
        std::cout << "step=" << (step + 1)
                  << " total_water_depth=" << std::fixed << std::setprecision(4)
                  << grid.total_water_depth() << " m\n";
    }

    print_water_depth(grid);
    return 0;
}

