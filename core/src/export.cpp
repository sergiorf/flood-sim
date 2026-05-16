#include "floodsim/export.hpp"

#include <iomanip>
#include <ostream>

namespace floodsim {

GridSummaryMetrics compute_grid_summary_metrics(const Grid& grid) {
    GridSummaryMetrics metrics;
    metrics.total_water_depth_m = grid.total_water_depth();

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            if (!grid.is_cell_valid(row, col)) {
                continue;
            }

            const double water_depth_m = grid.water_depth(row, col);
            if (water_depth_m <= 0.0) {
                continue;
            }

            ++metrics.wet_cell_count;
            if (!metrics.deepest_row.has_value() || water_depth_m > metrics.max_water_depth_m) {
                metrics.max_water_depth_m = water_depth_m;
                metrics.deepest_row = row;
                metrics.deepest_col = col;
            }
        }
    }

    return metrics;
}

void write_grid_csv(const Grid& grid, std::ostream& output, const GridCsvMetadata& metadata) {
    output << std::fixed << std::setprecision(6);
    output << "# floodsim_csv_version,1\n";
    output << "# rows," << grid.rows() << '\n';
    output << "# cols," << grid.cols() << '\n';
    output << "# cell_size_m," << grid.cell_size_m() << '\n';
    if (metadata.scenario_name.has_value()) {
        output << "# scenario_name," << *metadata.scenario_name << '\n';
    }
    if (metadata.boundary_mode.has_value()) {
        output << "# boundary_mode," << *metadata.boundary_mode << '\n';
    }
    if (metadata.rainfall_mode.has_value()) {
        output << "# rainfall_mode," << *metadata.rainfall_mode << '\n';
    }
    if (metadata.rainfall_profile_path.has_value()) {
        output << "# rainfall_profile_path," << *metadata.rainfall_profile_path << '\n';
    }
    if (metadata.surface_class_file.has_value()) {
        output << "# surface_class_file," << *metadata.surface_class_file << '\n';
    }
    if (metadata.impervious_cell_count.has_value()) {
        output << "# impervious_cell_count," << *metadata.impervious_cell_count << '\n';
    }
    if (metadata.impervious_runoff_coefficient.has_value()) {
        output << "# impervious_runoff_coefficient," << *metadata.impervious_runoff_coefficient << '\n';
    }
    if (metadata.impervious_initial_loss_m.has_value()) {
        output << "# impervious_initial_loss_m," << *metadata.impervious_initial_loss_m << '\n';
    }
    if (metadata.area_name.has_value()) {
        output << "# area_name," << *metadata.area_name << '\n';
    }
    if (metadata.area_source_name.has_value()) {
        output << "# area_source_name," << *metadata.area_source_name << '\n';
    }
    if (metadata.area_source_details.has_value()) {
        output << "# area_source_details," << *metadata.area_source_details << '\n';
    }
    if (metadata.area_source_kind.has_value()) {
        output << "# area_source_kind," << *metadata.area_source_kind << '\n';
    }
    if (metadata.area_source_url.has_value()) {
        output << "# area_source_url," << *metadata.area_source_url << '\n';
    }
    if (metadata.area_license_name.has_value()) {
        output << "# area_license_name," << *metadata.area_license_name << '\n';
    }
    if (metadata.area_cache_key.has_value()) {
        output << "# area_cache_key," << *metadata.area_cache_key << '\n';
    }
    if (metadata.area_boundary_path.has_value()) {
        output << "# area_boundary_path," << *metadata.area_boundary_path << '\n';
    }
    if (metadata.rainfall_intensity_m_per_hour.has_value()) {
        output << "# rainfall_intensity_m_per_hour," << *metadata.rainfall_intensity_m_per_hour << '\n';
    }
    if (metadata.peak_rainfall_intensity_m_per_hour.has_value()) {
        output << "# peak_rainfall_intensity_m_per_hour," << *metadata.peak_rainfall_intensity_m_per_hour << '\n';
    }
    if (metadata.total_rainfall_depth_m.has_value()) {
        output << "# total_rainfall_depth_m," << *metadata.total_rainfall_depth_m << '\n';
    }
    if (metadata.runoff_coefficient.has_value()) {
        output << "# runoff_coefficient," << *metadata.runoff_coefficient << '\n';
    }
    if (metadata.initial_loss_m.has_value()) {
        output << "# initial_loss_m," << *metadata.initial_loss_m << '\n';
    }
    if (metadata.time_step_seconds.has_value()) {
        output << "# time_step_seconds," << *metadata.time_step_seconds << '\n';
    }
    if (metadata.total_duration_seconds.has_value()) {
        output << "# total_duration_seconds," << *metadata.total_duration_seconds << '\n';
    }
    if (metadata.origin_x_m.has_value() && metadata.origin_y_m.has_value()) {
        output << "# origin_x_m," << *metadata.origin_x_m << '\n';
        output << "# origin_y_m," << *metadata.origin_y_m << '\n';
    }
    if (metadata.crs_id.has_value()) {
        output << "# crs_id," << *metadata.crs_id << '\n';
    }
    output << "row,col,elevation_m,water_depth_m,surface_height_m\n";

    for (std::size_t row = 0; row < grid.rows(); ++row) {
        for (std::size_t col = 0; col < grid.cols(); ++col) {
            output << row << ','
                   << col << ','
                   << grid.elevation(row, col) << ','
                   << grid.water_depth(row, col) << ','
                   << grid.surface_height(row, col) << '\n';
        }
    }
}

}  // namespace floodsim
