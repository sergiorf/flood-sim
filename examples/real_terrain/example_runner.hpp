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

// Narrow time-series rainfall contract for the real-terrain example.
//
// Each entry is the uniform rainfall intensity for one simulation step. The
// example keeps the time step constant across the full run, so realistic
// event shape comes from changing intensity over steps rather than from a full
// general scheduler.
struct RainfallProfile {
    std::filesystem::path source_path;
    std::vector<double> step_intensities_m_per_hour;
};

// One fully resolved scenario passed into the example runner.
//
// This is intentionally a small, explicit contract rather than a generalized
// scenario-management layer. Values may come from presets, scenario files, or
// CLI overrides, but run_example() receives a single normalized structure.
struct ScenarioConfig {
    std::string name;
    std::filesystem::path output_csv_path;
    double rainfall_intensity_m_per_hour {0.012};
    std::optional<RainfallProfile> rainfall_profile;
    double runoff_coefficient {1.0};
    double initial_loss_m {0.0};
    double time_step_seconds {300.0};
    int step_count {12};
    floodsim::BoundaryMode boundary_mode {floodsim::BoundaryMode::Open};
    bool preset_applied {false};
    bool file_applied {false};
    bool cli_overrides_applied {false};
};

// Optional CLI overrides applied after presets or external scenario
// definitions are loaded.
struct ScenarioOverrides {
    std::optional<double> rainfall_intensity_m_per_hour;
    std::optional<RainfallProfile> rainfall_profile;
    std::optional<double> runoff_coefficient;
    std::optional<double> initial_loss_m;
    std::optional<double> time_step_seconds;
    std::optional<int> step_count;
    std::optional<floodsim::BoundaryMode> boundary_mode;
};

// Narrow area-loading contract used by --area-file.
struct AreaDefinition {
    std::string area_name;
    std::filesystem::path contract_path;
    std::filesystem::path input_dem_path;
    std::string source_name;
    std::string source_details;
    bool external_source {false};
    std::optional<std::string> source_kind;
    std::optional<std::string> source_url;
    std::optional<std::string> license_name;
    std::optional<std::string> cache_key;
    std::optional<std::filesystem::path> staged_input_dem_path;
    std::optional<std::filesystem::path> cached_input_dem_path;
    std::optional<std::string> cache_status;
    std::optional<std::filesystem::path> boundary_path;
    std::optional<floodsim::TerrainWindow> terrain_window;
};

// Parsed command-line arguments plus any loaded scenario/area contracts.
struct ExampleArguments {
    std::filesystem::path input_dem_path;
    std::optional<AreaDefinition> area_definition;
    ScenarioConfig scenario;
    std::optional<std::filesystem::path> cache_dir;
    std::optional<floodsim::TerrainWindow> terrain_window;
    std::optional<int> snapshot_every_steps;
    ScenarioOverrides scenario_overrides;
    std::optional<std::filesystem::path> scenario_file_path;
    std::vector<ScenarioConfig> scenario_definitions;
    std::vector<std::string> batch_scenario_names;
};

struct ScenarioPreset {
    std::string_view name;
    double rainfall_intensity_m_per_hour;
    double time_step_seconds;
    int step_count;
    std::string_view description;
};

// Full output of one example run, including optional intermediate snapshots.
struct ExampleRunResult {
    struct SnapshotResult {
        int completed_steps;
        double elapsed_seconds;
        floodsim::Grid grid;
        floodsim::GridSummaryMetrics summary_metrics;
    };

    floodsim::LoadedTerrainRaster loaded_terrain;
    floodsim::Grid grid;
    floodsim::GridSummaryMetrics summary_metrics;
    std::vector<SnapshotResult> snapshots;
};

struct BatchScenarioResult {
    ExampleArguments arguments;
    ExampleRunResult result;
};

[[nodiscard]] std::string usage_message();
[[nodiscard]] std::string nodata_status_to_string(floodsim::TerrainNodataStatus status);
[[nodiscard]] std::string boundary_mode_to_string(floodsim::BoundaryMode mode);
[[nodiscard]] std::string scenario_source_to_string(const ScenarioConfig& scenario);
[[nodiscard]] std::string rainfall_mode_to_string(const ScenarioConfig& scenario);
[[nodiscard]] double scenario_peak_rainfall_intensity_m_per_hour(const ScenarioConfig& scenario);
[[nodiscard]] double scenario_total_rainfall_depth_m(const ScenarioConfig& scenario);

[[nodiscard]] const std::vector<ScenarioPreset>& scenario_presets();
[[nodiscard]] ExampleArguments parse_arguments(const std::vector<std::string>& args);
[[nodiscard]] ExampleArguments parse_arguments(int argc, char** argv);
[[nodiscard]] std::vector<ExampleArguments> build_batch_scenario_arguments(const ExampleArguments& arguments);
[[nodiscard]] std::filesystem::path derive_batch_comparison_output_path(const std::filesystem::path& base_output_path);
[[nodiscard]] std::filesystem::path derive_snapshot_output_path(
    const std::filesystem::path& base_output_path,
    int completed_steps,
    double elapsed_seconds);

[[nodiscard]] ExampleRunResult run_example(const ExampleArguments& arguments);
void write_export(
    const floodsim::Grid& grid,
    const floodsim::TerrainRaster& terrain,
    const std::optional<AreaDefinition>& area_definition,
    const ScenarioConfig& scenario,
    const std::filesystem::path& output_path);
void print_run_report(
    std::ostream& output,
    const ExampleArguments& arguments,
    const ExampleRunResult& result);
void write_snapshot_exports(
    const ExampleRunResult& result,
    const std::optional<AreaDefinition>& area_definition,
    const ScenarioConfig& scenario,
    const std::filesystem::path& base_output_path);
void write_batch_comparison_csv(
    const std::vector<BatchScenarioResult>& batch_results,
    const std::filesystem::path& output_path);

}  // namespace floodsim::examples::real_terrain
