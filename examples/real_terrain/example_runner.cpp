#include "example_runner.hpp"

#include <stdexcept>

namespace floodsim::examples::real_terrain {

namespace {

constexpr const char* kDefaultScenarioName = "baseline";

const ScenarioPreset& find_scenario_preset(const std::string& name) {
    for (const ScenarioPreset& preset : scenario_presets()) {
        if (preset.name == name) {
            return preset;
        }
    }

    throw std::runtime_error(
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

void validate_scenario_config(const ScenarioConfig& scenario) {
    if (scenario.name.empty()) {
        throw std::runtime_error("Scenario name must not be empty");
    }
    if (scenario.rainfall_intensity_m_per_hour < 0.0) {
        throw std::runtime_error("Rainfall intensity must be non-negative");
    }
    if (scenario.runoff_coefficient < 0.0 || scenario.runoff_coefficient > 1.0) {
        throw std::runtime_error("Runoff coefficient must be in [0, 1]");
    }
    if (scenario.time_step_seconds <= 0.0) {
        throw std::runtime_error("Time step must be positive");
    }
    if (scenario.step_count <= 0) {
        throw std::runtime_error("Step count must be positive");
    }
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

}  // namespace

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
        const int completed_steps = step + 1;
        if (arguments.snapshot_every_steps.has_value() &&
            completed_steps % *arguments.snapshot_every_steps == 0 &&
            completed_steps < arguments.scenario.step_count) {
            floodsim::Grid snapshot_grid = result.grid;
            result.snapshots.push_back(
                ExampleRunResult::SnapshotResult {
                    .completed_steps = completed_steps,
                    .elapsed_seconds = arguments.scenario.time_step_seconds * static_cast<double>(completed_steps),
                    .grid = std::move(snapshot_grid),
                    .summary_metrics = floodsim::compute_grid_summary_metrics(result.grid),
                });
        }
    }

    result.summary_metrics = floodsim::compute_grid_summary_metrics(result.grid);
    return result;
}

}  // namespace floodsim::examples::real_terrain
