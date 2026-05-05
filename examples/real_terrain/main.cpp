#include "floodsim/export.hpp"
#include "floodsim/simulation.hpp"
#include "floodsim/terrain.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

constexpr double kDefaultRainfallIntensityMPerHour = 0.012;
constexpr double kDefaultTimeStepSeconds = 300.0;
constexpr int kDefaultStepCount = 12;

struct ExampleArguments {
    std::filesystem::path input_dem_path;
    std::filesystem::path output_csv_path;
    double rainfall_intensity_m_per_hour {kDefaultRainfallIntensityMPerHour};
    double time_step_seconds {kDefaultTimeStepSeconds};
    int step_count {kDefaultStepCount};
};

[[noreturn]] void throw_usage_error(const std::string& message) {
    throw std::runtime_error(
        message +
        "\nUsage: floodsim_real_terrain_example <input_dem.tif> <output.csv>"
        " [--rainfall-intensity-m-per-hour <value>]"
        " [--time-step-seconds <value>]"
        " [--steps <count>]");
}

double parse_double_argument(const std::string& option, const std::string& value) {
    std::size_t parsed_length = 0;
    const double parsed_value = std::stod(value, &parsed_length);
    if (parsed_length != value.size()) {
        throw_usage_error("Invalid value for " + option + ": '" + value + "'");
    }
    return parsed_value;
}

int parse_int_argument(const std::string& option, const std::string& value) {
    std::size_t parsed_length = 0;
    const long parsed_value = std::stol(value, &parsed_length);
    if (parsed_length != value.size()) {
        throw_usage_error("Invalid value for " + option + ": '" + value + "'");
    }
    if (parsed_value < std::numeric_limits<int>::min() ||
        parsed_value > std::numeric_limits<int>::max()) {
        throw_usage_error("Value out of range for " + option + ": '" + value + "'");
    }
    return static_cast<int>(parsed_value);
}

ExampleArguments parse_arguments(int argc, char** argv) {
    if (argc < 3) {
        throw_usage_error("Missing required arguments");
    }

    ExampleArguments arguments {
        .input_dem_path = argv[1],
        .output_csv_path = argv[2],
    };

    for (int index = 3; index < argc; ++index) {
        const std::string option = argv[index];
        if (index + 1 >= argc) {
            throw_usage_error("Missing value for " + option);
        }

        const std::string value = argv[++index];
        if (option == "--rainfall-intensity-m-per-hour") {
            arguments.rainfall_intensity_m_per_hour = parse_double_argument(option, value);
        } else if (option == "--time-step-seconds") {
            arguments.time_step_seconds = parse_double_argument(option, value);
        } else if (option == "--steps") {
            arguments.step_count = parse_int_argument(option, value);
        } else {
            throw_usage_error("Unknown option: " + option);
        }
    }

    if (arguments.rainfall_intensity_m_per_hour < 0.0) {
        throw_usage_error("Rainfall intensity must be non-negative");
    }
    if (arguments.time_step_seconds <= 0.0) {
        throw_usage_error("Time step must be positive");
    }
    if (arguments.step_count <= 0) {
        throw_usage_error("Step count must be positive");
    }

    return arguments;
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
    try {
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
            .intensity_m_per_hour = arguments.rainfall_intensity_m_per_hour,
        };
        const floodsim::SimulationConfig config {
            .time_step_seconds = arguments.time_step_seconds,
            .max_outflow_fraction = 0.20,
            .boundary_mode = floodsim::BoundaryMode::Closed,
        };

        for (int step = 0; step < arguments.step_count; ++step) {
            floodsim::step(grid, rainfall, config);
        }

        write_export(grid, terrain, arguments.output_csv_path);

        std::cout << "rainfall_intensity_m_per_hour=" << std::fixed << std::setprecision(6)
                  << arguments.rainfall_intensity_m_per_hour << '\n';
        std::cout << "time_step_seconds=" << std::fixed << std::setprecision(3)
                  << arguments.time_step_seconds << '\n';
        std::cout << "steps=" << arguments.step_count
                  << " total_water_depth_m=" << std::fixed << std::setprecision(6)
                  << grid.total_water_depth() << '\n';
        std::cout << "wrote_csv=" << arguments.output_csv_path << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
