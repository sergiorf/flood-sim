#include "example_runner.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace floodsim::examples::real_terrain {

namespace {

std::string batch_output_stem(const std::filesystem::path& base_output_path) {
    const std::string stem = base_output_path.stem().string();
    return stem.empty() ? base_output_path.filename().string() : stem;
}

}  // namespace

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

std::string rainfall_mode_to_string(const ScenarioConfig& scenario) {
    return scenario.rainfall_profile.has_value() ? "profile" : "uniform";
}

double scenario_peak_rainfall_intensity_m_per_hour(const ScenarioConfig& scenario) {
    if (!scenario.rainfall_profile.has_value()) {
        return scenario.rainfall_intensity_m_per_hour;
    }

    double peak_intensity = 0.0;
    for (const double intensity : scenario.rainfall_profile->step_intensities_m_per_hour) {
        if (intensity > peak_intensity) {
            peak_intensity = intensity;
        }
    }
    return peak_intensity;
}

double scenario_total_rainfall_depth_m(const ScenarioConfig& scenario) {
    const double seconds_per_hour = 3600.0;
    if (!scenario.rainfall_profile.has_value()) {
        return scenario.rainfall_intensity_m_per_hour *
            (scenario.time_step_seconds / seconds_per_hour) *
            static_cast<double>(scenario.step_count);
    }

    double total_depth_m = 0.0;
    for (const double intensity : scenario.rainfall_profile->step_intensities_m_per_hour) {
        total_depth_m += intensity * (scenario.time_step_seconds / seconds_per_hour);
    }
    return total_depth_m;
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

std::filesystem::path derive_batch_comparison_output_path(const std::filesystem::path& base_output_path) {
    const std::filesystem::path parent = base_output_path.parent_path();
    return parent / (batch_output_stem(base_output_path) + "_comparison.csv");
}

std::filesystem::path derive_snapshot_output_path(
    const std::filesystem::path& base_output_path,
    int completed_steps,
    double elapsed_seconds) {
    const std::filesystem::path parent = base_output_path.parent_path();
    const std::string extension = base_output_path.has_extension()
        ? base_output_path.extension().string()
        : ".csv";
    std::ostringstream step_fragment;
    step_fragment << std::setw(4) << std::setfill('0') << completed_steps;
    const long elapsed_seconds_rounded = std::lround(elapsed_seconds);
    return parent /
        (batch_output_stem(base_output_path) + "_step" + step_fragment.str() +
         "_t" + std::to_string(elapsed_seconds_rounded) + "s" + extension);
}

void write_export(
    const floodsim::Grid& grid,
    const floodsim::TerrainRaster& terrain,
    const std::optional<AreaDefinition>& area_definition,
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
            .rainfall_mode = rainfall_mode_to_string(scenario),
            .rainfall_profile_path = scenario.rainfall_profile.has_value()
                ? std::optional<std::string>(scenario.rainfall_profile->source_path.string())
                : std::nullopt,
            .area_name = area_definition.has_value()
                ? std::optional<std::string>(area_definition->area_name)
                : std::nullopt,
            .area_source_name = area_definition.has_value()
                ? std::optional<std::string>(area_definition->source_name)
                : std::nullopt,
            .area_source_details = area_definition.has_value()
                ? std::optional<std::string>(area_definition->source_details)
                : std::nullopt,
            .area_boundary_path = (area_definition.has_value() && area_definition->boundary_path.has_value())
                ? std::optional<std::string>(area_definition->boundary_path->string())
                : std::nullopt,
            .rainfall_intensity_m_per_hour = scenario.rainfall_profile.has_value()
                ? std::nullopt
                : std::optional<double>(scenario.rainfall_intensity_m_per_hour),
            .peak_rainfall_intensity_m_per_hour = scenario_peak_rainfall_intensity_m_per_hour(scenario),
            .total_rainfall_depth_m = scenario_total_rainfall_depth_m(scenario),
            .runoff_coefficient = scenario.runoff_coefficient,
            .initial_loss_m = scenario.initial_loss_m,
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
    if (arguments.area_definition.has_value()) {
        const AreaDefinition& area_definition = *arguments.area_definition;
        output << "area_name=" << area_definition.area_name << '\n';
        output << "area_contract_path=" << area_definition.contract_path << '\n';
        output << "area_source_name=" << area_definition.source_name << '\n';
        output << "area_source_details=" << area_definition.source_details << '\n';
        if (area_definition.boundary_path.has_value()) {
            output << "area_boundary_path=" << *area_definition.boundary_path << '\n';
        }
    }
    output << "boundary_mode=" << boundary_mode_to_string(scenario.boundary_mode) << '\n';
    output << "rainfall_mode=" << rainfall_mode_to_string(scenario) << '\n';
    if (scenario.rainfall_profile.has_value()) {
        output << "rainfall_profile_path=" << scenario.rainfall_profile->source_path << '\n';
        output << "peak_rainfall_intensity_m_per_hour=" << std::fixed << std::setprecision(6)
               << scenario_peak_rainfall_intensity_m_per_hour(scenario) << '\n';
        output << "total_rainfall_depth_m=" << std::fixed << std::setprecision(6)
               << scenario_total_rainfall_depth_m(scenario) << '\n';
    }
    output << "runoff_coefficient=" << std::fixed << std::setprecision(6)
           << scenario.runoff_coefficient << '\n';
    output << "initial_loss_m=" << std::fixed << std::setprecision(6)
           << scenario.initial_loss_m << '\n';
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

    if (!scenario.rainfall_profile.has_value()) {
        output << "rainfall_intensity_m_per_hour=" << std::fixed << std::setprecision(6)
               << scenario.rainfall_intensity_m_per_hour << '\n';
        output << "total_rainfall_depth_m=" << std::fixed << std::setprecision(6)
               << scenario_total_rainfall_depth_m(scenario) << '\n';
    }
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
    for (const auto& snapshot : result.snapshots) {
        output << "snapshot_metrics"
               << " completed_steps=" << snapshot.completed_steps
               << " elapsed_seconds=" << std::fixed << std::setprecision(3)
               << snapshot.elapsed_seconds
               << " wet_cells=" << snapshot.summary_metrics.wet_cell_count
               << " max_water_depth_m=" << std::fixed << std::setprecision(6)
               << snapshot.summary_metrics.max_water_depth_m
               << '\n';
    }
}

void write_snapshot_exports(
    const ExampleRunResult& result,
    const std::optional<AreaDefinition>& area_definition,
    const ScenarioConfig& scenario,
    const std::filesystem::path& base_output_path) {
    for (const auto& snapshot : result.snapshots) {
        const std::filesystem::path snapshot_output_path =
            derive_snapshot_output_path(base_output_path, snapshot.completed_steps, snapshot.elapsed_seconds);
        write_export(
            snapshot.grid,
            result.loaded_terrain.terrain,
            area_definition,
            ScenarioConfig {
                .name = scenario.name,
                .output_csv_path = snapshot_output_path,
                .rainfall_intensity_m_per_hour = scenario.rainfall_intensity_m_per_hour,
                .runoff_coefficient = scenario.runoff_coefficient,
                .initial_loss_m = scenario.initial_loss_m,
                .time_step_seconds = scenario.time_step_seconds,
                .step_count = snapshot.completed_steps,
                .boundary_mode = scenario.boundary_mode,
            },
            snapshot_output_path);
    }
}

void write_batch_comparison_csv(
    const std::vector<BatchScenarioResult>& batch_results,
    const std::filesystem::path& output_path) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("Failed to open batch comparison CSV output path");
    }

    output << "scenario_name,boundary_mode,rainfall_mode,rainfall_intensity_m_per_hour,rainfall_profile_path,"
              "peak_rainfall_intensity_m_per_hour,total_rainfall_depth_m,runoff_coefficient,initial_loss_m,"
              "time_step_seconds,steps,total_water_depth_m,wet_cells,max_water_depth_m,"
              "deepest_row,deepest_col,output_csv\n";

    for (const BatchScenarioResult& batch_result : batch_results) {
        const ScenarioConfig& scenario = batch_result.arguments.scenario;
        const floodsim::GridSummaryMetrics& metrics = batch_result.result.summary_metrics;
        output << scenario.name << ','
               << boundary_mode_to_string(scenario.boundary_mode) << ','
               << rainfall_mode_to_string(scenario) << ','
               << std::fixed << std::setprecision(6);
        if (!scenario.rainfall_profile.has_value()) {
            output << scenario.rainfall_intensity_m_per_hour;
        }
        output << ',';
        if (scenario.rainfall_profile.has_value()) {
            output << scenario.rainfall_profile->source_path.string();
        }
        output << ','
               << scenario_peak_rainfall_intensity_m_per_hour(scenario) << ','
               << scenario_total_rainfall_depth_m(scenario) << ','
               << scenario.runoff_coefficient << ','
               << scenario.initial_loss_m << ','
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
