#include "example_runner.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <stdexcept>

namespace floodsim::examples::real_terrain {

namespace {

constexpr const char* kDefaultScenarioName = "baseline";

[[noreturn]] void throw_usage_error(const std::string& message) {
    throw std::runtime_error(message + "\n" + usage_message());
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

floodsim::BoundaryMode parse_boundary_mode_argument(const std::string& value) {
    if (value == "closed") {
        return floodsim::BoundaryMode::Closed;
    }
    if (value == "open") {
        return floodsim::BoundaryMode::Open;
    }

    throw_usage_error("Invalid value for --boundary-mode: '" + value + "'");
}

const ScenarioPreset& find_scenario_preset(const std::string& name) {
    for (const ScenarioPreset& preset : scenario_presets()) {
        if (preset.name == name) {
            return preset;
        }
    }

    throw_usage_error(
        "Unknown scenario preset: " + name +
        ". Expected one of: baseline, intense_short, long_moderate");
}

void apply_scenario_preset(ScenarioConfig& scenario, const ScenarioPreset& preset) {
    scenario.name = std::string(preset.name);
    scenario.rainfall_intensity_m_per_hour = preset.rainfall_intensity_m_per_hour;
    scenario.time_step_seconds = preset.time_step_seconds;
    scenario.step_count = preset.step_count;
    scenario.preset_applied = true;
}

void validate_scenario_config(const ScenarioConfig& scenario) {
    if (scenario.rainfall_intensity_m_per_hour < 0.0) {
        throw_usage_error("Rainfall intensity must be non-negative");
    }
    if (scenario.runoff_coefficient < 0.0 || scenario.runoff_coefficient > 1.0) {
        throw_usage_error("Runoff coefficient must be in [0, 1]");
    }
    if (scenario.time_step_seconds <= 0.0) {
        throw_usage_error("Time step must be positive");
    }
    if (scenario.step_count <= 0) {
        throw_usage_error("Step count must be positive");
    }
}

void validate_window_arguments(const std::optional<floodsim::TerrainWindow>& terrain_window) {
    if (!terrain_window.has_value()) {
        return;
    }

    const floodsim::TerrainWindow& window = *terrain_window;
    const bool has_any_window_field =
        window.row_offset != 0 || window.col_offset != 0 || window.rows != 0 || window.cols != 0;
    const bool has_complete_window = window.rows != 0 && window.cols != 0;
    if (has_any_window_field && !has_complete_window) {
        throw_usage_error(
            "Terrain window requires both --window-rows and --window-cols when any window option is used");
    }
}

}  // namespace

std::string usage_message() {
    return
        "Usage: floodsim_real_terrain_example <input_dem.tif> <output.csv>"
        " [--scenario <name>]"
        " [--boundary-mode <closed|open>]"
        " [--rainfall-intensity-m-per-hour <value>]"
        " [--runoff-coefficient <value>]"
        " [--time-step-seconds <value>]"
        " [--steps <count>]"
        " [--window-row-offset <value>]"
        " [--window-col-offset <value>]"
        " [--window-rows <value>]"
        " [--window-cols <value>]";
}

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

std::string boundary_mode_to_string(floodsim::BoundaryMode mode) {
    switch (mode) {
        case floodsim::BoundaryMode::Closed:
            return "closed";
        case floodsim::BoundaryMode::Open:
            return "open";
    }

    throw std::runtime_error("Unhandled boundary mode");
}

std::string scenario_source_to_string(const ScenarioConfig& scenario) {
    if (scenario.preset_applied && scenario.cli_overrides_applied) {
        return "preset_with_cli_overrides";
    }
    if (scenario.preset_applied) {
        return "preset";
    }
    return "direct_cli_or_default";
}

const std::vector<ScenarioPreset>& scenario_presets() {
    static const std::vector<ScenarioPreset> presets = {
        {
            .name = "baseline",
            .rainfall_intensity_m_per_hour = 0.012,
            .time_step_seconds = 300.0,
            .step_count = 12,
            .description = "Moderate one-hour event at 12 mm/hour.",
        },
        {
            .name = "intense_short",
            .rainfall_intensity_m_per_hour = 0.030,
            .time_step_seconds = 300.0,
            .step_count = 6,
            .description = "Short 30-minute burst at 30 mm/hour.",
        },
        {
            .name = "long_moderate",
            .rainfall_intensity_m_per_hour = 0.008,
            .time_step_seconds = 300.0,
            .step_count = 36,
            .description = "Longer three-hour event at 8 mm/hour.",
        },
    };
    return presets;
}

ExampleArguments parse_arguments(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        throw_usage_error("Missing required arguments");
    }

    ExampleArguments arguments {
        .input_dem_path = args[1],
        .scenario =
            ScenarioConfig {
                .name = kDefaultScenarioName,
                .output_csv_path = args[2],
            },
    };
    std::optional<std::string> scenario_preset_name;
    std::optional<double> rainfall_override;
    std::optional<double> runoff_coefficient_override;
    std::optional<double> time_step_override;
    std::optional<int> step_count_override;

    for (std::size_t index = 3; index < args.size(); ++index) {
        const std::string& option = args[index];
        if (index + 1 >= args.size()) {
            throw_usage_error("Missing value for " + option);
        }

        const std::string& value = args[++index];
        if (option == "--scenario") {
            scenario_preset_name = value;
        } else if (option == "--boundary-mode") {
            arguments.scenario.boundary_mode = parse_boundary_mode_argument(value);
        } else if (option == "--rainfall-intensity-m-per-hour") {
            rainfall_override = parse_double_argument(option, value);
        } else if (option == "--runoff-coefficient") {
            runoff_coefficient_override = parse_double_argument(option, value);
        } else if (option == "--time-step-seconds") {
            time_step_override = parse_double_argument(option, value);
        } else if (option == "--steps") {
            step_count_override = parse_int_argument(option, value);
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

    if (scenario_preset_name.has_value()) {
        apply_scenario_preset(arguments.scenario, find_scenario_preset(*scenario_preset_name));
    }
    if (rainfall_override.has_value()) {
        arguments.scenario.rainfall_intensity_m_per_hour = *rainfall_override;
    }
    if (runoff_coefficient_override.has_value()) {
        arguments.scenario.runoff_coefficient = *runoff_coefficient_override;
    }
    if (time_step_override.has_value()) {
        arguments.scenario.time_step_seconds = *time_step_override;
    }
    if (step_count_override.has_value()) {
        arguments.scenario.step_count = *step_count_override;
    }
    arguments.scenario.cli_overrides_applied =
        arguments.scenario.preset_applied &&
        (rainfall_override.has_value() || runoff_coefficient_override.has_value() ||
         time_step_override.has_value() || step_count_override.has_value());

    validate_scenario_config(arguments.scenario);
    validate_window_arguments(arguments.terrain_window);
    return arguments;
}

ExampleArguments parse_arguments(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int index = 0; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }
    return parse_arguments(args);
}

ExampleRunResult run_example(const ExampleArguments& arguments) {
    ExampleRunResult result {
        .loaded_terrain = arguments.terrain_window.has_value()
            ? floodsim::load_terrain_raster_with_report(arguments.input_dem_path.string(), *arguments.terrain_window)
            : floodsim::load_terrain_raster_with_report(arguments.input_dem_path.string()),
        .grid = floodsim::Grid(1, 1),
    };
    result.grid = floodsim::make_grid_from_terrain(result.loaded_terrain.terrain);

    const floodsim::RainfallScenario rainfall {
        .intensity_m_per_hour = arguments.scenario.rainfall_intensity_m_per_hour,
    };
    const floodsim::SimulationConfig config {
        .time_step_seconds = arguments.scenario.time_step_seconds,
        .runoff_coefficient = arguments.scenario.runoff_coefficient,
        .max_outflow_fraction = 0.20,
        .boundary_mode = arguments.scenario.boundary_mode,
    };

    for (int step = 0; step < arguments.scenario.step_count; ++step) {
        floodsim::step(result.grid, rainfall, config);
    }

    result.summary_metrics = floodsim::compute_grid_summary_metrics(result.grid);
    return result;
}

void write_export(
    const floodsim::Grid& grid,
    const floodsim::TerrainRaster& terrain,
    const ScenarioConfig& scenario,
    const std::filesystem::path& output_path) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("Failed to open CSV output path");
    }

    floodsim::write_grid_csv(
        grid,
        output,
        floodsim::GridCsvMetadata {
            .scenario_name = scenario.name,
            .boundary_mode = boundary_mode_to_string(scenario.boundary_mode),
            .rainfall_intensity_m_per_hour = scenario.rainfall_intensity_m_per_hour,
            .runoff_coefficient = scenario.runoff_coefficient,
            .time_step_seconds = scenario.time_step_seconds,
            .total_duration_seconds = scenario.time_step_seconds * static_cast<double>(scenario.step_count),
            .origin_x_m = terrain.origin_x_m,
            .origin_y_m = terrain.origin_y_m,
            .crs_id = terrain.crs_id,
        });
}

void print_run_report(
    std::ostream& output,
    const ExampleArguments& arguments,
    const ExampleRunResult& result) {
    const floodsim::TerrainRaster& terrain = result.loaded_terrain.terrain;
    const floodsim::TerrainIngestionReport& ingestion_report = result.loaded_terrain.report;
    const ScenarioConfig& scenario = arguments.scenario;

    output << "loaded_dem=" << arguments.input_dem_path << '\n';
    output << "scenario_name=" << scenario.name
           << " scenario_source=" << scenario_source_to_string(scenario) << '\n';
    output << "boundary_mode=" << boundary_mode_to_string(scenario.boundary_mode) << '\n';
    output << "runoff_coefficient=" << std::fixed << std::setprecision(6)
           << scenario.runoff_coefficient << '\n';
    output << "rows=" << terrain.rows
           << " cols=" << terrain.cols
           << " cell_size_m=" << std::fixed << std::setprecision(3)
           << terrain.cell_size_m
           << " valid_cells=" << terrain.valid_cell_count() << '\n';
    output << "ingestion_report"
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
        output << "crs=" << terrain.crs_id.value() << '\n';
    }
    if (arguments.terrain_window.has_value()) {
        const floodsim::TerrainWindow& window = *arguments.terrain_window;
        output << "window_row_offset=" << window.row_offset
               << " window_col_offset=" << window.col_offset
               << " window_rows=" << window.rows
               << " window_cols=" << window.cols << '\n';
    }

    output << "rainfall_intensity_m_per_hour=" << std::fixed << std::setprecision(6)
           << scenario.rainfall_intensity_m_per_hour << '\n';
    output << "time_step_seconds=" << std::fixed << std::setprecision(3)
           << scenario.time_step_seconds << '\n';
    output << "steps=" << scenario.step_count
           << " total_water_depth_m=" << std::fixed << std::setprecision(6)
           << result.grid.total_water_depth() << '\n';
    output << "summary_metrics"
           << " wet_cells=" << result.summary_metrics.wet_cell_count
           << " max_water_depth_m=" << std::fixed << std::setprecision(6)
           << result.summary_metrics.max_water_depth_m
           << " deepest_row=";
    if (result.summary_metrics.deepest_row.has_value()) {
        output << *result.summary_metrics.deepest_row;
    } else {
        output << "none";
    }
    output << " deepest_col=";
    if (result.summary_metrics.deepest_col.has_value()) {
        output << *result.summary_metrics.deepest_col;
    } else {
        output << "none";
    }
    output << '\n';
    output << "wrote_csv=" << scenario.output_csv_path << '\n';
}

}  // namespace floodsim::examples::real_terrain
