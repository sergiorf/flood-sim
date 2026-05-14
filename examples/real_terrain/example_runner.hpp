#pragma once

#include "floodsim/export.hpp"
#include "floodsim/simulation.hpp"
#include "floodsim/terrain.hpp"

#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace floodsim::examples::real_terrain {

struct ScenarioConfig {
    std::string name;
    std::filesystem::path output_csv_path;
    double rainfall_intensity_m_per_hour {0.012};
    double runoff_coefficient {1.0};
    double time_step_seconds {300.0};
    int step_count {12};
    floodsim::BoundaryMode boundary_mode {floodsim::BoundaryMode::Open};
    bool preset_applied {false};
    bool cli_overrides_applied {false};
};

struct ScenarioOverrides {
    std::optional<double> rainfall_intensity_m_per_hour;
    std::optional<double> runoff_coefficient;
    std::optional<double> time_step_seconds;
    std::optional<int> step_count;
    std::optional<floodsim::BoundaryMode> boundary_mode;
};

struct ExampleArguments {
    std::filesystem::path input_dem_path;
    ScenarioConfig scenario;
    std::optional<floodsim::TerrainWindow> terrain_window;
    ScenarioOverrides scenario_overrides;
    std::vector<std::string> batch_scenario_names;
};

struct ScenarioPreset {
    std::string_view name;
    double rainfall_intensity_m_per_hour;
    double time_step_seconds;
    int step_count;
    std::string_view description;
};

struct ExampleRunResult {
    floodsim::LoadedTerrainRaster loaded_terrain;
    floodsim::Grid grid;
    floodsim::GridSummaryMetrics summary_metrics;
};

[[nodiscard]] std::string usage_message();
[[nodiscard]] std::string nodata_status_to_string(floodsim::TerrainNodataStatus status);
[[nodiscard]] std::string boundary_mode_to_string(floodsim::BoundaryMode mode);
[[nodiscard]] std::string scenario_source_to_string(const ScenarioConfig& scenario);

[[nodiscard]] const std::vector<ScenarioPreset>& scenario_presets();
[[nodiscard]] ExampleArguments parse_arguments(const std::vector<std::string>& args);
[[nodiscard]] ExampleArguments parse_arguments(int argc, char** argv);
[[nodiscard]] std::vector<ExampleArguments> build_batch_scenario_arguments(const ExampleArguments& arguments);

[[nodiscard]] ExampleRunResult run_example(const ExampleArguments& arguments);
void write_export(
    const floodsim::Grid& grid,
    const floodsim::TerrainRaster& terrain,
    const ScenarioConfig& scenario,
    const std::filesystem::path& output_path);
void print_run_report(
    std::ostream& output,
    const ExampleArguments& arguments,
    const ExampleRunResult& result);

}  // namespace floodsim::examples::real_terrain
