#include "floodsim/export.hpp"
#include "floodsim/simulation.hpp"
#include "floodsim/terrain.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct ExampleArguments {
    std::filesystem::path input_dem_path;
    std::filesystem::path output_csv_path;
};

ExampleArguments parse_arguments(int argc, char** argv) {
    if (argc != 3) {
        throw std::runtime_error(
            "Usage: floodsim_real_terrain_example <input_dem.tif> <output.csv>");
    }

    return ExampleArguments {
        .input_dem_path = argv[1],
        .output_csv_path = argv[2],
    };
}

void write_export(
    const floodsim::Grid& grid,
    const floodsim::TerrainRaster& terrain,
    const std::filesystem::path& output_path) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("Failed to open CSV output path");
    }

    floodsim::write_grid_csv(
        grid,
        output,
        floodsim::GridCsvMetadata {
            .origin_x_m = terrain.origin_x_m,
            .origin_y_m = terrain.origin_y_m,
            .crs_id = terrain.crs_id,
        });
}

}  // namespace

int main(int argc, char** argv) {
    const ExampleArguments arguments = parse_arguments(argc, argv);
    const floodsim::TerrainRaster terrain =
        floodsim::load_terrain_raster_from_file(arguments.input_dem_path.string());
    floodsim::Grid grid = floodsim::make_grid_from_terrain(terrain);

    std::cout << "loaded_dem=" << arguments.input_dem_path << '\n';
    std::cout << "rows=" << terrain.rows
              << " cols=" << terrain.cols
              << " cell_size_m=" << std::fixed << std::setprecision(3)
              << terrain.cell_size_m
              << " valid_cells=" << terrain.valid_cell_count() << '\n';
    if (terrain.crs_id.has_value()) {
        std::cout << "crs=" << terrain.crs_id.value() << '\n';
    }

    const floodsim::RainfallScenario rainfall {
        .intensity_m_per_hour = 0.012,
    };
    const floodsim::SimulationConfig config {
        .time_step_seconds = 300.0,
        .max_outflow_fraction = 0.20,
        .boundary_mode = floodsim::BoundaryMode::Closed,
    };

    constexpr int kStepCount = 12;
    for (int step = 0; step < kStepCount; ++step) {
        floodsim::step(grid, rainfall, config);
    }

    write_export(grid, terrain, arguments.output_csv_path);

    std::cout << "steps=" << kStepCount
              << " total_water_depth_m=" << std::fixed << std::setprecision(6)
              << grid.total_water_depth() << '\n';
    std::cout << "wrote_csv=" << arguments.output_csv_path << '\n';
    return 0;
}
