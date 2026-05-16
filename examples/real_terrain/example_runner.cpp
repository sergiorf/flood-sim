#include "example_runner.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace floodsim::examples::real_terrain {

namespace {

std::vector<std::uint8_t> build_impervious_mask(
    const SurfaceClassConfig& surface_class_config,
    const floodsim::Grid& grid) {
    std::vector<std::uint8_t> impervious_mask(grid.rows() * grid.cols(), 0);
    for (const SurfaceClassCell& cell : surface_class_config.impervious_cells) {
        if (cell.row >= grid.rows() || cell.col >= grid.cols()) {
            throw std::runtime_error("Surface class cell is outside the loaded terrain extent");
        }
        if (!grid.is_cell_valid(cell.row, cell.col)) {
            throw std::runtime_error("Surface class cell targets an invalid or nodata terrain cell");
        }
        impervious_mask[cell.row * grid.cols() + cell.col] = 1;
    }
    return impervious_mask;
}

void initialize_surface_class_initial_loss(
    floodsim::Grid& grid,
    const std::vector<std::uint8_t>& impervious_mask,
    double pervious_initial_loss_m,
    double impervious_initial_loss_m) {
    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            if (!grid.is_cell_valid(row, col)) {
                continue;
            }

            const std::size_t idx = row * grid.cols() + col;
            const double initial_loss_m =
                impervious_mask[idx] != 0 ? impervious_initial_loss_m : pervious_initial_loss_m;
            grid.set_initial_loss_remaining(row, col, initial_loss_m);
        }
    }
}

void add_surface_class_rainfall(
    floodsim::Grid& grid,
    const floodsim::RainfallScenario& rainfall,
    double duration_seconds,
    double pervious_runoff_coefficient,
    double pervious_initial_loss_m,
    const SurfaceClassConfig& surface_class_config,
    const std::vector<std::uint8_t>& impervious_mask) {
    if (duration_seconds < 0.0) {
        throw std::invalid_argument("Rainfall duration cannot be negative");
    }
    if (pervious_runoff_coefficient < 0.0 || pervious_runoff_coefficient > 1.0) {
        throw std::invalid_argument("Runoff coefficient must be in [0, 1]");
    }
    if (pervious_initial_loss_m < 0.0) {
        throw std::invalid_argument("Initial loss must be non-negative");
    }
    if (surface_class_config.impervious_runoff_coefficient < 0.0 ||
        surface_class_config.impervious_runoff_coefficient > 1.0) {
        throw std::invalid_argument("Impervious runoff coefficient must be in [0, 1]");
    }
    if (surface_class_config.impervious_initial_loss_m < 0.0) {
        throw std::invalid_argument("Impervious initial loss must be non-negative");
    }

    if ((pervious_initial_loss_m > 0.0 || surface_class_config.impervious_initial_loss_m > 0.0) &&
        !grid.initial_loss_initialized()) {
        initialize_surface_class_initial_loss(
            grid,
            impervious_mask,
            pervious_initial_loss_m,
            surface_class_config.impervious_initial_loss_m);
    }

    const double gross_depth = rainfall.intensity_m_per_hour * (duration_seconds / 3600.0);
    if (gross_depth <= 0.0) {
        return;
    }

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            if (!grid.is_cell_valid(row, col)) {
                continue;
            }

            const std::size_t idx = row * grid.cols() + col;
            const double runoff_coefficient =
                impervious_mask[idx] != 0
                    ? surface_class_config.impervious_runoff_coefficient
                    : pervious_runoff_coefficient;
            const double post_initial_loss_depth =
                grid.consume_initial_loss(row, col, gross_depth);
            const double retained_depth = post_initial_loss_depth * runoff_coefficient;
            if (retained_depth > 0.0) {
                grid.add_water_depth(row, col, retained_depth);
            }
        }
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
                has_cli_scenario_overrides(arguments.scenario_overrides);
            validate_scenario_config(scenario_arguments.scenario);
            batch_arguments.push_back(std::move(scenario_arguments));
        }
        return batch_arguments;
    }

    for (const std::string& scenario_name : arguments.batch_scenario_names) {
        ExampleArguments scenario_arguments = arguments;
        scenario_arguments.batch_scenario_names.clear();
        scenario_arguments.scenario = ScenarioConfig {
            .name = default_scenario_name(),
            .output_csv_path = derive_batch_output_path(arguments.scenario.output_csv_path, scenario_name),
        };

        apply_scenario_preset(scenario_arguments.scenario, find_scenario_preset(scenario_name));
        apply_scenario_overrides(scenario_arguments.scenario, arguments.scenario_overrides);

        scenario_arguments.scenario.cli_overrides_applied =
            has_cli_scenario_overrides(arguments.scenario_overrides);
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

    const floodsim::SimulationConfig config {
        .time_step_seconds = arguments.scenario.time_step_seconds,
        .runoff_coefficient = arguments.scenario.runoff_coefficient,
        .initial_loss_m = arguments.scenario.initial_loss_m,
        .max_outflow_fraction = 0.20,
        .boundary_mode = arguments.scenario.boundary_mode,
    };
    const std::vector<std::uint8_t> impervious_mask = arguments.surface_class_config.has_value()
        ? build_impervious_mask(*arguments.surface_class_config, result.grid)
        : std::vector<std::uint8_t> {};

    for (int step = 0; step < arguments.scenario.step_count; ++step) {
        const double rainfall_intensity_m_per_hour = arguments.scenario.rainfall_profile.has_value()
            ? arguments.scenario.rainfall_profile->step_intensities_m_per_hour[static_cast<std::size_t>(step)]
            : arguments.scenario.rainfall_intensity_m_per_hour;
        const floodsim::RainfallScenario rainfall {
            .intensity_m_per_hour = rainfall_intensity_m_per_hour,
        };
        if (arguments.surface_class_config.has_value()) {
            add_surface_class_rainfall(
                result.grid,
                rainfall,
                config.time_step_seconds,
                arguments.scenario.runoff_coefficient,
                arguments.scenario.initial_loss_m,
                *arguments.surface_class_config,
                impervious_mask);
            floodsim::route_surface_water(result.grid, config);
        } else {
            floodsim::step(result.grid, rainfall, config);
        }
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
