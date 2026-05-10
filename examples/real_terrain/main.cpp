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

struct ScenarioConfig {
    std::filesystem::path output_csv_path;
    double rainfall_intensity_m_per_hour {kDefaultRainfallIntensityMPerHour};
    double time_step_seconds {kDefaultTimeStepSeconds};
    int step_count {kDefaultStepCount};
};

struct ExampleArguments {
    std::filesystem::path input_dem_path;
    ScenarioConfig scenario;
    std::optional<floodsim::TerrainWindow> terrain_window;
};

std::string nodata_status_to_string(floodsim::TerrainNodataStatus status) {
    switch (status) {
        case floodsim::TerrainNodataStatus::BandMetadataApplied:
            return "band_metadata_applied";
        case floodsim::TerrainNodataStatus::BandMetadataMissingAllCellsValid:
            return "band_metadata_missing_all_cells_valid";
        case floodsim::TerrainNodataStatus::BandMetadataMissingNaNCellsPresent:
            return "band_metadata_missing_nan_cells_present";
    }

    throw std::runtime_error("Unhandled terrain nodata status");
}

[[noreturn]] void throw_usage_error(const std::string& message) {
    throw std::runtime_error(
        message +
        "\nUsage: floodsim_real_terrain_example <input_dem.tif> <output.csv>"
        " [--rainfall-intensity-m-per-hour <value>]"
        " [--time-step-seconds <value>]"
        " [--steps <count>]"
        " [--window-row-offset <value>]"
        " [--window-col-offset <value>]"
        " [--window-rows <value>]"
        " [--window-cols <value>]");
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

void validate_scenario_config(const ScenarioConfig& scenario) {
    if (scenario.rainfall_intensity_m_per_hour < 0.0) {
        throw_usage_error("Rainfall intensity must be non-negative");
    }
    if (scenario.time_step_seconds <= 0.0) {
        throw_usage_error("Time step must be positive");
    }
    if (scenario.step_count <= 0) {
        throw_usage_error("Step count must be positive");
    }
}

ExampleArguments parse_arguments(int argc, char** argv) {
    if (argc < 3) {
        throw_usage_error("Missing required arguments");
    }

    ExampleArguments arguments {
        .input_dem_path = argv[1],
        .scenario =
            ScenarioConfig {
                .output_csv_path = argv[2],
            },
    };

    for (int index = 3; index < argc; ++index) {
        const std::string option = argv[index];
        if (index + 1 >= argc) {
            throw_usage_error("Missing value for " + option);
        }

        const std::string value = argv[++index];
        if (option == "--rainfall-intensity-m-per-hour") {
            arguments.scenario.rainfall_intensity_m_per_hour = parse_double_argument(option, value);
        } else if (option == "--time-step-seconds") {
            arguments.scenario.time_step_seconds = parse_double_argument(option, value);
        } else if (option == "--steps") {
            arguments.scenario.step_count = parse_int_argument(option, value);
        } else if (option == "--window-row-offset") {
            if (!arguments.terrain_window.has_value()) {
                arguments.terrain_window = floodsim::TerrainWindow {};
            }
            const int parsed_value = parse_int_argument(option, value);
            if (parsed_value < 0) {
                throw_usage_error("Terrain window row offset must be non-negative");
            }
            arguments.terrain_window->row_offset = static_cast<std::size_t>(parsed_value);
        } else if (option == "--window-col-offset") {
            if (!arguments.terrain_window.has_value()) {
                arguments.terrain_window = floodsim::TerrainWindow {};
            }
            const int parsed_value = parse_int_argument(option, value);
            if (parsed_value < 0) {
                throw_usage_error("Terrain window column offset must be non-negative");
            }
            arguments.terrain_window->col_offset = static_cast<std::size_t>(parsed_value);
        } else if (option == "--window-rows") {
            if (!arguments.terrain_window.has_value()) {
                arguments.terrain_window = floodsim::TerrainWindow {};
            }
            const int parsed_value = parse_int_argument(option, value);
            if (parsed_value <= 0) {
                throw_usage_error("Terrain window rows must be positive");
            }
            arguments.terrain_window->rows = static_cast<std::size_t>(parsed_value);
        } else if (option == "--window-cols") {
            if (!arguments.terrain_window.has_value()) {
                arguments.terrain_window = floodsim::TerrainWindow {};
            }
            const int parsed_value = parse_int_argument(option, value);
            if (parsed_value <= 0) {
                throw_usage_error("Terrain window columns must be positive");
            }
            arguments.terrain_window->cols = static_cast<std::size_t>(parsed_value);
        } else {
            throw_usage_error("Unknown option: " + option);
        }
    }

    validate_scenario_config(arguments.scenario);
    if (arguments.terrain_window.has_value()) {
        const floodsim::TerrainWindow& window = *arguments.terrain_window;
        const bool has_any_window_field =
            window.row_offset != 0 || window.col_offset != 0 || window.rows != 0 || window.cols != 0;
        const bool has_complete_window =
            window.rows != 0 && window.cols != 0;
        if (has_any_window_field && !has_complete_window) {
            throw_usage_error(
                "Terrain window requires both --window-rows and --window-cols when any window option is used");
        }
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
        const ScenarioConfig& scenario = arguments.scenario;
        const floodsim::LoadedTerrainRaster loaded_terrain = arguments.terrain_window.has_value()
            ? floodsim::load_terrain_raster_with_report(
                  arguments.input_dem_path.string(),
                  *arguments.terrain_window)
            : floodsim::load_terrain_raster_with_report(arguments.input_dem_path.string());
        const floodsim::TerrainRaster& terrain = loaded_terrain.terrain;
        const floodsim::TerrainIngestionReport& ingestion_report = loaded_terrain.report;
        floodsim::Grid grid = floodsim::make_grid_from_terrain(terrain);

        std::cout << "loaded_dem=" << arguments.input_dem_path << '\n';
        std::cout << "rows=" << terrain.rows
                  << " cols=" << terrain.cols
                  << " cell_size_m=" << std::fixed << std::setprecision(3)
                  << terrain.cell_size_m
                  << " valid_cells=" << terrain.valid_cell_count() << '\n';
        std::cout << "ingestion_report"
                  << " source_rows=" << ingestion_report.source_rows
                  << " source_cols=" << ingestion_report.source_cols
                  << " loaded_rows=" << ingestion_report.loaded_rows
                  << " loaded_cols=" << ingestion_report.loaded_cols
                  << " clipped_cells=" << ingestion_report.clipped_cell_count
                  << " invalid_cells=" << ingestion_report.invalid_cell_count
                  << " nodata_metadata_present=" << (ingestion_report.nodata_metadata_present ? "true" : "false")
                  << " nan_cells=" << ingestion_report.nan_cell_count
                  << " nodata_status=" << nodata_status_to_string(ingestion_report.nodata_status)
                  << '\n';
        if (terrain.crs_id.has_value()) {
            std::cout << "crs=" << terrain.crs_id.value() << '\n';
        }
        if (arguments.terrain_window.has_value()) {
            const floodsim::TerrainWindow& window = *arguments.terrain_window;
            std::cout << "window_row_offset=" << window.row_offset
                      << " window_col_offset=" << window.col_offset
                      << " window_rows=" << window.rows
                      << " window_cols=" << window.cols << '\n';
        }

        const floodsim::RainfallScenario rainfall {
            .intensity_m_per_hour = scenario.rainfall_intensity_m_per_hour,
        };
        const floodsim::SimulationConfig config {
            .time_step_seconds = scenario.time_step_seconds,
            .max_outflow_fraction = 0.20,
            .boundary_mode = floodsim::BoundaryMode::Closed,
        };

        for (int step = 0; step < scenario.step_count; ++step) {
            floodsim::step(grid, rainfall, config);
        }

        write_export(grid, terrain, scenario.output_csv_path);

        std::cout << "rainfall_intensity_m_per_hour=" << std::fixed << std::setprecision(6)
                  << scenario.rainfall_intensity_m_per_hour << '\n';
        std::cout << "time_step_seconds=" << std::fixed << std::setprecision(3)
                  << scenario.time_step_seconds << '\n';
        std::cout << "steps=" << scenario.step_count
                  << " total_water_depth_m=" << std::fixed << std::setprecision(6)
                  << grid.total_water_depth() << '\n';
        std::cout << "wrote_csv=" << scenario.output_csv_path << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
