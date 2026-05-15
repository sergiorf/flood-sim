#include "example_runner.hpp"

#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace floodsim::examples::real_terrain {

namespace {

constexpr const char* kDefaultScenarioName = "baseline";
constexpr std::string_view kScenarioFileHeader =
    "scenario_name,rainfall_intensity_m_per_hour,runoff_coefficient,initial_loss_m,time_step_seconds,steps,boundary_mode";
constexpr std::string_view kAreaFileHeader =
    "area_name,input_dem_path,window_row_offset,window_col_offset,window_rows,window_cols,source_name,source_details,boundary_path";

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

std::filesystem::path resolve_contract_path(
    const std::filesystem::path& contract_path,
    const std::string& field_value) {
    const std::filesystem::path parsed_path(field_value);
    if (parsed_path.is_absolute()) {
        return parsed_path;
    }
    return contract_path.parent_path() / parsed_path;
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
    if (scenario.initial_loss_m < 0.0) {
        throw_usage_error("Initial loss must be non-negative");
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
    if (overrides.initial_loss_m.has_value()) {
        scenario.initial_loss_m = *overrides.initial_loss_m;
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

void validate_snapshot_interval(const std::optional<int> snapshot_every_steps) {
    if (snapshot_every_steps.has_value() && *snapshot_every_steps <= 0) {
        throw_usage_error("Snapshot interval must be positive");
    }
}

void validate_area_definition(const AreaDefinition& area_definition) {
    if (area_definition.area_name.empty()) {
        throw_usage_error("Area definition must provide a non-empty area_name");
    }
    if (area_definition.input_dem_path.empty()) {
        throw_usage_error("Area definition must provide a non-empty input_dem_path");
    }
    if (area_definition.source_name.empty()) {
        throw_usage_error("Area definition must provide a non-empty source_name");
    }
    if (area_definition.source_details.empty()) {
        throw_usage_error("Area definition must provide a non-empty source_details");
    }
    if (area_definition.terrain_window.has_value()) {
        validate_window_arguments(area_definition.terrain_window);
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

AreaDefinition load_area_definition(const std::filesystem::path& area_file_path) {
    std::ifstream input(area_file_path);
    if (!input) {
        throw_usage_error("Failed to open area file: '" + area_file_path.string() + "'");
    }

    std::string header_line;
    if (!std::getline(input, header_line)) {
        throw_usage_error("Area file is empty: '" + area_file_path.string() + "'");
    }
    if (trim_copy(header_line) != kAreaFileHeader) {
        throw_usage_error(
            "Area file has invalid header in '" + area_file_path.string() +
            "'. Expected: " + std::string(kAreaFileHeader));
    }

    std::vector<std::string> non_empty_rows;
    std::string line;
    while (std::getline(input, line)) {
        if (!trim_copy(line).empty()) {
            non_empty_rows.push_back(line);
        }
    }

    if (non_empty_rows.empty()) {
        throw_usage_error("Area file does not contain any area rows: '" + area_file_path.string() + "'");
    }
    if (non_empty_rows.size() != 1) {
        throw_usage_error("Area file must contain exactly one non-empty area row: '" + area_file_path.string() + "'");
    }

    std::vector<std::string> fields = split_csv_line(non_empty_rows.front());
    if (fields.size() == 8 && !non_empty_rows.front().empty() && non_empty_rows.front().back() == ',') {
        fields.push_back("");
    }
    if (fields.size() != 9) {
        throw_usage_error(
            "Area file row in '" + area_file_path.string() +
            "' must contain exactly 9 comma-separated fields");
    }

    AreaDefinition area_definition;
    area_definition.contract_path = area_file_path;
    area_definition.area_name = fields[0];
    area_definition.input_dem_path = resolve_contract_path(area_file_path, fields[1]);
    area_definition.source_name = fields[6];
    area_definition.source_details = fields[7];
    if (!fields[8].empty()) {
        area_definition.boundary_path = resolve_contract_path(area_file_path, fields[8]);
    }

    const bool has_any_window_field =
        !fields[2].empty() || !fields[3].empty() || !fields[4].empty() || !fields[5].empty();
    const bool has_complete_window =
        !fields[2].empty() && !fields[3].empty() && !fields[4].empty() && !fields[5].empty();
    if (has_any_window_field && !has_complete_window) {
        throw_usage_error(
            "Area file window fields must provide row_offset, col_offset, rows, and cols together");
    }
    if (has_complete_window) {
        floodsim::TerrainWindow window;
        const int row_offset = parse_int_argument("area file window_row_offset", fields[2]);
        const int col_offset = parse_int_argument("area file window_col_offset", fields[3]);
        const int rows = parse_int_argument("area file window_rows", fields[4]);
        const int cols = parse_int_argument("area file window_cols", fields[5]);
        if (row_offset < 0 || col_offset < 0) {
            throw_usage_error("Area file window offsets must be non-negative");
        }
        if (rows <= 0 || cols <= 0) {
            throw_usage_error("Area file window rows and cols must be positive");
        }
        window.row_offset = static_cast<std::size_t>(row_offset);
        window.col_offset = static_cast<std::size_t>(col_offset);
        window.rows = static_cast<std::size_t>(rows);
        window.cols = static_cast<std::size_t>(cols);
        area_definition.terrain_window = window;
    }

    validate_area_definition(area_definition);
    return area_definition;
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
        if (fields.size() != 7) {
            throw_usage_error(
                "Scenario file row " + std::to_string(line_number) + " in '" +
                scenario_file_path.string() + "' must contain exactly 7 comma-separated fields");
        }

        ScenarioConfig scenario;
        scenario.name = fields[0];
        scenario.rainfall_intensity_m_per_hour =
            parse_double_argument("scenario file rainfall intensity", fields[1]);
        scenario.runoff_coefficient =
            parse_double_argument("scenario file runoff coefficient", fields[2]);
        scenario.initial_loss_m =
            parse_double_argument("scenario file initial loss", fields[3]);
        scenario.time_step_seconds =
            parse_double_argument("scenario file time step", fields[4]);
        scenario.step_count =
            parse_int_argument("scenario file step count", fields[5]);
        scenario.boundary_mode =
            parse_boundary_mode_value(fields[6], "scenario file boundary mode");
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
        " | floodsim_real_terrain_example --area-file <path.csv> <output.csv>"
        " [--scenario <name>]"
        " [--batch-scenarios <name1,name2,...>]"
        " [--scenario-file <path.csv>]"
        " [--snapshot-every-steps <count>]"
        " [--boundary-mode <closed|open>]"
        " [--rainfall-intensity-m-per-hour <value>]"
        " [--runoff-coefficient <value>]"
        " [--initial-loss-m <value>]"
        " [--time-step-seconds <value>]"
        " [--steps <count>]"
        " [--window-row-offset <value>]"
        " [--window-col-offset <value>]"
        " [--window-rows <value>]"
        " [--window-cols <value>]";
}

ExampleArguments parse_arguments(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw_usage_error("Missing required arguments");
    }

    ExampleArguments arguments {
        .scenario =
            ScenarioConfig {
                .name = kDefaultScenarioName,
            },
    };
    std::size_t index = 1;
    if (args[index] == "--area-file") {
        if (args.size() < 4) {
            throw_usage_error("Area-file mode requires --area-file <path.csv> <output.csv>");
        }
        const std::filesystem::path area_file_path = args[index + 1];
        arguments.area_definition = load_area_definition(area_file_path);
        arguments.input_dem_path = arguments.area_definition->input_dem_path;
        arguments.terrain_window = arguments.area_definition->terrain_window;
        arguments.scenario.output_csv_path = args[index + 2];
        index += 3;
    } else {
        if (args.size() < 3) {
            throw_usage_error("Missing required arguments");
        }
        arguments.input_dem_path = args[index];
        arguments.scenario.output_csv_path = args[index + 1];
        index += 2;
    }

    std::optional<std::string> scenario_preset_name;
    std::optional<std::vector<std::string>> batch_scenario_names;
    std::optional<std::filesystem::path> scenario_file_path;
    std::optional<int> snapshot_every_steps;
    std::optional<double> rainfall_override;
    std::optional<double> runoff_coefficient_override;
    std::optional<double> initial_loss_override;
    std::optional<double> time_step_override;
    std::optional<int> step_count_override;

    for (; index < args.size(); ++index) {
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
        } else if (option == "--snapshot-every-steps") {
            snapshot_every_steps = parse_int_argument(option, value);
            arguments.snapshot_every_steps = snapshot_every_steps;
        } else if (option == "--boundary-mode") {
            arguments.scenario.boundary_mode = parse_boundary_mode_argument(value);
            arguments.scenario_overrides.boundary_mode = arguments.scenario.boundary_mode;
        } else if (option == "--rainfall-intensity-m-per-hour") {
            rainfall_override = parse_double_argument(option, value);
            arguments.scenario_overrides.rainfall_intensity_m_per_hour = rainfall_override;
        } else if (option == "--runoff-coefficient") {
            runoff_coefficient_override = parse_double_argument(option, value);
            arguments.scenario_overrides.runoff_coefficient = runoff_coefficient_override;
        } else if (option == "--initial-loss-m") {
            initial_loss_override = parse_double_argument(option, value);
            arguments.scenario_overrides.initial_loss_m = initial_loss_override;
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
            const std::filesystem::path output_csv_path = arguments.scenario.output_csv_path;
            arguments.scenario = arguments.scenario_definitions.front();
            arguments.scenario.output_csv_path = output_csv_path;
        }
    }
    if (rainfall_override.has_value()) {
        arguments.scenario.rainfall_intensity_m_per_hour = *rainfall_override;
    }
    if (runoff_coefficient_override.has_value()) {
        arguments.scenario.runoff_coefficient = *runoff_coefficient_override;
    }
    if (initial_loss_override.has_value()) {
        arguments.scenario.initial_loss_m = *initial_loss_override;
    }
    if (time_step_override.has_value()) {
        arguments.scenario.time_step_seconds = *time_step_override;
    }
    if (step_count_override.has_value()) {
        arguments.scenario.step_count = *step_count_override;
    }
    const bool has_cli_scenario_overrides =
        rainfall_override.has_value() || runoff_coefficient_override.has_value() ||
        initial_loss_override.has_value() ||
        time_step_override.has_value() || step_count_override.has_value();
    arguments.scenario.cli_overrides_applied =
        (arguments.scenario.preset_applied || arguments.scenario.file_applied) &&
        has_cli_scenario_overrides;

    validate_scenario_config(arguments.scenario);
    validate_window_arguments(arguments.terrain_window);
    validate_snapshot_interval(arguments.snapshot_every_steps);
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

}  // namespace floodsim::examples::real_terrain
