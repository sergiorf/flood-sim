#include "floodsim/export.hpp"
#include "floodsim/grid.hpp"
#include "floodsim/simulation.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

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

void export_grid_if_requested(const floodsim::Grid& grid, int argc, char** argv) {
    if (argc < 2) {
        return;
    }

    const char* output_path = argv[1];
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("Failed to open CSV output path");
    }

    floodsim::write_grid_csv(grid, output);
    std::cout << "wrote_csv=" << output_path << '\n';
}

}  // namespace

int main(int argc, char** argv) {
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
    export_grid_if_requested(grid, argc, argv);
    return 0;
}
