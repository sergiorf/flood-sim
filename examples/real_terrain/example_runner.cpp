#include "example_runner.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace floodsim::examples::real_terrain {

namespace {

constexpr const char* kDefaultScenarioName = "baseline";
constexpr std::string_view kScenarioFileHeader =
    "scenario_name,rainfall_intensity_m_per_hour,runoff_coefficient,time_step_seconds,steps,boundary_mode";

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

floodsim::BoundaryMode parse_boundary_mode_value(const std::string& value, const std::string& context) {
    if (value == "closed") {
        return floodsim::BoundaryMode::Closed;
    }
    if (value == "open") {
        return floodsim::BoundaryMode::Open;
    }

    throw_usage_error("Invalid value for " + context + ": '" + value + "'");
}

floodsim::BoundaryMode parse_boundary_mode_argument(const std::string& value) {
    return parse_boundary_mode_value(value, "--boundary-mode");
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

std::string trim_copy(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = value.find_last_not_of(" \t\r");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream line_stream(line);
    std::string field;
    while (std::getline(line_stream, field, ',')) {
        fields.push_back(trim_copy(field));
    }
    return fields;
}

void validate_scenario_config(const ScenarioConfig& scenario) {
    if (scenario.name.empty()) {
        throw_usage_error("Scenario name must not be empty");
    }
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

void apply_scenario_overrides(ScenarioConfig& scenario, const ScenarioOverrides& overrides) {
    if (overrides.boundary_mode.has_value()) {
        scenario.boundary_mode = *overrides.boundary_mode;
    }
    if (overrides.rainfall_intensity_m_per_hour.has_value()) {
        scenario.rainfall_intensity_m_per_hour = *overrides.rainfall_intensity_m_per_hour;
    }
    if (overrides.runoff_coefficient.has_value()) {
        scenario.runoff_coefficient = *overrides.runoff_coefficient;
    }
    if (overrides.time_step_seconds.has_value()) {
        scenario.time_step_seconds = *overrides.time_step_seconds;
    }
    if (overrides.step_count.has_value()) {
        scenario.step_count = *overrides.step_count;
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

std::vector<std::string> parse_batch_scenario_names_argument(const std::string& value) {
    std::vector<std::string> names;
    std::stringstream value_stream(value);
    std::string item;
    while (std::getline(value_stream, item, ',')) {
        if (item.empty()) {
            throw_usage_error("Batch scenario list cannot contain empty names");
        }
        names.push_back(item);
    }

    if (names.empty()) {
        throw_usage_error("Batch scenario list cannot be empty");
    }

    return names;
}

std::filesystem::path derive_batch_output_path(
    const std::filesystem::path& base_output_path,
    const std::string& scenario_name) {
    const std::filesystem::path parent = base_output_path.parent_path();
    const std::string stem = base_output_path.stem().string();
    const std::string extension = base_output_path.has_extension()
        ? base_output_path.extension().string()
        : ".csv";
    const std::string batch_stem = stem.empty()
        ? base_output_path.filename().string()
        : stem;
    return parent / (batch_stem + "_" + scenario_name + extension);
}

std::string batch_output_stem(const std::filesystem::path& base_output_path) {
    const std::string stem = base_output_path.stem().string();
    return stem.empty() ? base_output_path.filename().string() : stem;
}

std::vector<ScenarioConfig> load_scenario_file_definitions(const std::filesystem::path& scenario_file_path) {
    std::ifstream input(scenario_file_path);
    if (!input) {
        throw_usage_error("Failed to open scenario file: '" + scenario_file_path.string() + "'");
    }

    std::string header_line;
    if (!std::getline(input, header_line)) {
        throw_usage_error("Scenario file is empty: '" + scenario_file_path.string() + "'");
    }
    if (trim_copy(header_line) != kScenarioFileHeader) {
        throw_usage_error(
            "Scenario file has invalid header in '" + scenario_file_path.string() +
            "'. Expected: " + std::string(kScenarioFileHeader));
    }

    std::vector<ScenarioConfig> scenarios;
    std::string line;
    std::size_t line_number = 1;
    while (std::getline(input, line)) {
        ++line_number;
        if (trim_copy(line).empty()) {
            continue;
        }

        const std::vector<std::string> fields = split_csv_line(line);
        if (fields.size() != 6) {
            throw_usage_error(
                "Scenario file row " + std::to_string(line_number) + " in '" +
                scenario_file_path.string() + "' must contain exactly 6 comma-separated fields");
        }

        ScenarioConfig scenario;
        scenario.name = fields[0];
        scenario.rainfall_intensity_m_per_hour =
            parse_double_argument("scenario file rainfall intensity", fields[1]);
        scenario.runoff_coefficient =
            parse_double_argument("scenario file runoff coefficient", fields[2]);
        scenario.time_step_seconds =
            parse_double_argument("scenario file time step", fields[3]);
        scenario.step_count =
            parse_int_argument("scenario file step count", fields[4]);
        scenario.boundary_mode =
            parse_boundary_mode_value(fields[5], "scenario file boundary mode");
        scenario.file_applied = true;
        validate_scenario_config(scenario);
        scenarios.push_back(std::move(scenario));
    }

    if (scenarios.empty()) {
        throw_usage_error("Scenario file does not contain any scenario rows: '" + scenario_file_path.string() + "'");
    }

    return scenarios;
}

}  // namespace

std::string usage_message() {
    return
        "Usage: floodsim_real_terrain_example <input_dem.tif> <output.csv>"
        " [--scenario <name>]"
        " [--batch-scenarios <name1,name2,...>]"
        " [--scenario-file <path.csv>]"
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
    if (scenario.file_applied && scenario.cli_overrides_applied) {
        return "file_with_cli_overrides";
    }
    if (scenario.file_applied) {
        return "file";
    }
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
    std::optional<std::vector<std::string>> batch_scenario_names;
    std::optional<std::filesystem::path> scenario_file_path;
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
        } else if (option == "--batch-scenarios") {
            batch_scenario_names = parse_batch_scenario_names_argument(value);
        } else if (option == "--scenario-file") {
            scenario_file_path = value;
        } else if (option == "--boundary-mode") {
            arguments.scenario.boundary_mode = parse_boundary_mode_argument(value);
            arguments.scenario_overrides.boundary_mode = arguments.scenario.boundary_mode;
        } else if (option == "--rainfall-intensity-m-per-hour") {
            rainfall_override = parse_double_argument(option, value);
            arguments.scenario_overrides.rainfall_intensity_m_per_hour = rainfall_override;
        } else if (option == "--runoff-coefficient") {
            runoff_coefficient_override = parse_double_argument(option, value);
            arguments.scenario_overrides.runoff_coefficient = runoff_coefficient_override;
        } else if (option == "--time-step-seconds") {
            time_step_override = parse_double_argument(option, value);
            arguments.scenario_overrides.time_step_seconds = time_step_override;
        } else if (option == "--steps") {
            step_count_override = parse_int_argument(option, value);
            arguments.scenario_overrides.step_count = step_count_override;
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

    if ((scenario_preset_name.has_value() && batch_scenario_names.has_value()) ||
        (scenario_preset_name.has_value() && scenario_file_path.has_value()) ||
        (batch_scenario_names.has_value() && scenario_file_path.has_value())) {
        throw_usage_error("Use only one of --scenario, --batch-scenarios, or --scenario-file");
    }

    if (scenario_preset_name.has_value()) {
        apply_scenario_preset(arguments.scenario, find_scenario_preset(*scenario_preset_name));
    }
    if (batch_scenario_names.has_value()) {
        for (const std::string& name : *batch_scenario_names) {
            static_cast<void>(find_scenario_preset(name));
        }
        arguments.batch_scenario_names = *batch_scenario_names;
    }
    if (scenario_file_path.has_value()) {
        arguments.scenario_file_path = *scenario_file_path;
        arguments.scenario_definitions = load_scenario_file_definitions(*scenario_file_path);
        if (arguments.scenario_definitions.size() == 1) {
            arguments.scenario = arguments.scenario_definitions.front();
            arguments.scenario.output_csv_path = args[2];
        }
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
    const bool has_cli_scenario_overrides =
        rainfall_override.has_value() || runoff_coefficient_override.has_value() ||
        time_step_override.has_value() || step_count_override.has_value();
    arguments.scenario.cli_overrides_applied =
        (arguments.scenario.preset_applied || arguments.scenario.file_applied) &&
        has_cli_scenario_overrides;

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

std::vector<ExampleArguments> build_batch_scenario_arguments(const ExampleArguments& arguments) {
    std::vector<ExampleArguments> batch_arguments;
    const std::size_t batch_size =
        !arguments.scenario_definitions.empty() ? arguments.scenario_definitions.size() : arguments.batch_scenario_names.size();
    batch_arguments.reserve(batch_size);

    if (!arguments.scenario_definitions.empty()) {
        for (const ScenarioConfig& file_scenario : arguments.scenario_definitions) {
            ExampleArguments scenario_arguments = arguments;
            scenario_arguments.batch_scenario_names.clear();
            scenario_arguments.scenario_definitions.clear();
            scenario_arguments.scenario = file_scenario;
            scenario_arguments.scenario.output_csv_path =
                derive_batch_output_path(arguments.scenario.output_csv_path, file_scenario.name);
            apply_scenario_overrides(scenario_arguments.scenario, arguments.scenario_overrides);
            scenario_arguments.scenario.cli_overrides_applied =
                arguments.scenario_overrides.boundary_mode.has_value() ||
                arguments.scenario_overrides.rainfall_intensity_m_per_hour.has_value() ||
                arguments.scenario_overrides.runoff_coefficient.has_value() ||
                arguments.scenario_overrides.time_step_seconds.has_value() ||
                arguments.scenario_overrides.step_count.has_value();
            validate_scenario_config(scenario_arguments.scenario);
            batch_arguments.push_back(std::move(scenario_arguments));
        }
        return batch_arguments;
    }

    for (const std::string& scenario_name : arguments.batch_scenario_names) {
        ExampleArguments scenario_arguments = arguments;
        scenario_arguments.batch_scenario_names.clear();
        scenario_arguments.scenario = ScenarioConfig {
            .name = kDefaultScenarioName,
            .output_csv_path = derive_batch_output_path(arguments.scenario.output_csv_path, scenario_name),
        };

        apply_scenario_preset(scenario_arguments.scenario, find_scenario_preset(scenario_name));
        apply_scenario_overrides(scenario_arguments.scenario, arguments.scenario_overrides);

        scenario_arguments.scenario.cli_overrides_applied =
            arguments.scenario_overrides.boundary_mode.has_value() ||
            arguments.scenario_overrides.rainfall_intensity_m_per_hour.has_value() ||
            arguments.scenario_overrides.runoff_coefficient.has_value() ||
            arguments.scenario_overrides.time_step_seconds.has_value() ||
            arguments.scenario_overrides.step_count.has_value();
        validate_scenario_config(scenario_arguments.scenario);
        batch_arguments.push_back(std::move(scenario_arguments));
    }

    return batch_arguments;
}

std::filesystem::path derive_batch_comparison_output_path(const std::filesystem::path& base_output_path) {
    const std::filesystem::path parent = base_output_path.parent_path();
    return parent / (batch_output_stem(base_output_path) + "_comparison.csv");
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

void write_batch_comparison_csv(
    const std::vector<BatchScenarioResult>& batch_results,
    const std::filesystem::path& output_path) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("Failed to open batch comparison CSV output path");
    }

    output << "scenario_name,boundary_mode,rainfall_intensity_m_per_hour,runoff_coefficient,"
              "time_step_seconds,steps,total_water_depth_m,wet_cells,max_water_depth_m,"
              "deepest_row,deepest_col,output_csv\n";

    for (const BatchScenarioResult& batch_result : batch_results) {
        const ScenarioConfig& scenario = batch_result.arguments.scenario;
        const floodsim::GridSummaryMetrics& metrics = batch_result.result.summary_metrics;
        output << scenario.name << ','
               << boundary_mode_to_string(scenario.boundary_mode) << ','
               << std::fixed << std::setprecision(6)
               << scenario.rainfall_intensity_m_per_hour << ','
               << scenario.runoff_coefficient << ','
               << scenario.time_step_seconds << ','
               << scenario.step_count << ','
               << batch_result.result.grid.total_water_depth() << ','
               << metrics.wet_cell_count << ','
               << metrics.max_water_depth_m << ',';
        if (metrics.deepest_row.has_value()) {
            output << *metrics.deepest_row;
        } else {
            output << "none";
        }
        output << ',';
        if (metrics.deepest_col.has_value()) {
            output << *metrics.deepest_col;
        } else {
            output << "none";
        }
        output << ',' << scenario.output_csv_path.string() << '\n';
    }
}

}  // namespace floodsim::examples::real_terrain
