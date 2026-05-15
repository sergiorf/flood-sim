#pragma once

#include "floodsim/grid.hpp"

#include <iosfwd>
#include <optional>
#include <string>

namespace floodsim {

// Small summary metrics used for deterministic comparisons in tests, CLI
// output, and lightweight scenario reporting.
struct GridSummaryMetrics {
    double total_water_depth_m {0.0};
    double max_water_depth_m {0.0};
    std::size_t wet_cell_count {0};
    std::optional<std::size_t> deepest_row;
    std::optional<std::size_t> deepest_col;
};

// Optional metadata preamble for CSV exports.
//
// The export format stays row-per-cell and text-based, so scenario identity,
// timing, and terrain provenance are carried through this narrow sideband
// rather than a separate manifest.
struct GridCsvMetadata {
    std::optional<std::string> scenario_name;
    std::optional<std::string> boundary_mode;
    std::optional<std::string> rainfall_mode;
    std::optional<std::string> rainfall_profile_path;
    std::optional<std::string> area_name;
    std::optional<std::string> area_source_name;
    std::optional<std::string> area_source_details;
    std::optional<std::string> area_source_kind;
    std::optional<std::string> area_source_url;
    std::optional<std::string> area_license_name;
    std::optional<std::string> area_cache_key;
    std::optional<std::string> area_boundary_path;
    std::optional<double> rainfall_intensity_m_per_hour;
    std::optional<double> peak_rainfall_intensity_m_per_hour;
    std::optional<double> total_rainfall_depth_m;
    std::optional<double> runoff_coefficient;
    std::optional<double> initial_loss_m;
    std::optional<double> time_step_seconds;
    std::optional<double> total_duration_seconds;
    std::optional<double> origin_x_m;
    std::optional<double> origin_y_m;
    std::optional<std::string> crs_id;
};

// Write a text CSV export with a short metadata preamble followed by one data
// row per cell. The preamble keeps the Phase 1 output self-describing without
// introducing a separate sidecar file or binary container. Terrain-derived
// runs can attach scenario, timing, and origin / CRS metadata without changing
// the per-cell layout.
[[nodiscard]] GridSummaryMetrics compute_grid_summary_metrics(const Grid& grid);

void write_grid_csv(
    const Grid& grid,
    std::ostream& output,
    const GridCsvMetadata& metadata = {});

}  // namespace floodsim
