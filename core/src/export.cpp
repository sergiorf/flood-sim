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
    if (metadata.rainfall_intensity_m_per_hour.has_value()) {
        output << "# rainfall_intensity_m_per_hour," << *metadata.rainfall_intensity_m_per_hour << '\n';
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
